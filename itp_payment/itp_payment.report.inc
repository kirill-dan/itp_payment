<?php

/**
 * Module for create payment via different payment processors
 * Developer: Danilevsky Kirill k.danilevsky@gmail.com
 *
 * File for report about payment.
 *
 */

/**
 * Create page for payments report.
 */
function itp_payment_report_page() {

  // Create title for table.
  $header = array(
    array('data' => t('№'), 'field' => 'idr'),
    array('data' => t('ID in system'), 'field' => 'ind_system'),
    array('data' => t('Date'), 'field' => 'date', 'sort' => 'desc'),
    array('data' => t('User'), 'field' => 'uid'),
    array('data' => t('Node Type')),
    array('data' => t('Product'), 'field' => 'nid'),
    array('data' => t('Amount'), 'field' => 'amount'),
    array('data' => t('Period'), 'field' => 'period'),
    array('data' => t('Status'), 'field' => 'status'),
    array('data' => t('Processor')),
    array('data' => t('Payment name'), 'field' => 'name_pay'),
    array('data' => t('Active now')),
    array('data' => t('Payer')),
    array('data' => t('Change status')),
    array('data' => t('Delete')),
  );
  // Get orders data.
  $query = db_select('itp_payment_report', 'r')
    ->fields('r')
    ->extend('PagerDefault')
    ->limit(50)
    ->extend('TableSort')
    ->orderByHeader($header);
  $query->leftJoin('itp_payment_processors_settings', 's', 's.ids=r.ids');
  $query->leftJoin('itp_payment_processors', 'p', 'p.idp=s.idp');
  $query->fields('s', array('name_pay', 'payer'));
  $query->fields('p', array('name_processor'));
  $result = $query->execute();
  $rows = array();
  // Collect nodes.
  foreach ($result as $value) {
    $user = user_load($value->uid);
    $node = node_load($value->nid);

    if (!$value->period) {
      $period = t('Unlimited');
      $active = $value->status == 'Paid' ? '<span class="itp-color_status">' . t('Yes') . '</span>' : t('No');
    }
    else {
      $period = $value->period . ' ' . t('days');
      $sec_in_day = 60 * 60 * 24;
      // Checking expired whether date payment.
      if ($value->date + $value->period * $sec_in_day > time()) {
        $active = $value->status == 'Paid' ? '<span class="itp-color_status">' . t('Yes') . '</span>' : t('No');
      }
      else {
        $active = t('No');
      }
    }
    $user_data = user_view($user);
    $user_data = drupal_render($user_data);

    // Make link for delete pay.
    $form_del = '<div class="itp-payment-button-table">' . l(t('Delete'), 'admin/itp-payment/report/nojs/delete/' . $value->idr, array(
        'attributes' => array(
          'class' => array(
            'ctools-use-modal',
            'ctools-modal-modal-popup-small-size',
          )
        ),
      )) . '</div>';

    // Get form for change status payment.
    $form_get_change_status = drupal_get_form('itp_payment_report_change_status_form', $value->idr);
    $form_change_status = drupal_render($form_get_change_status);

    $user_info = '<div class="itp-payment-report-user"><div class="itp-payment-report-user-name">' .
      l($user->name, 'user/' . $value->uid, array(
        'attributes' => array(
          'target' => '_blank',
          'class' => 'itp-payment-report-user-link'
        )
      )) .
      '</div>
                  <div class="itp-payment-report-user-info">' . $user_data . '</div></div>';

    $rows[] = array(
      $value->idr,
      $value->ind_system,
      format_date($value->date, 'short'),
      $user_info,
      $node->type,
      l(mb_substr($node->title, 0, 30), 'node/' . $value->nid, array(
        'attributes' => array(
          'title' => $node->title,
          'target' => '_blank',
        )
      )),
      '$' . $value->amount,
      $period,
      $value->status,
      $value->name_processor,
      $value->name_pay,
      $active,
      $value->payer,
      $form_change_status,
      $form_del,
    );
  }

  $xls_form = drupal_get_form('itp_payment_report_csv_export_form');
  $title = '<div class="payments-title-report">' . t('List of those who made ​​a payment:') . '</div>
            <div class="payments-title-xls-button">' . drupal_render($xls_form) . ' </div > ';
  $output = $title . theme('table', array('header' => $header, 'rows' => $rows));
  $output .= theme('pager');
  return $output;
}

/**
 * Confirm delete form.
 *
 * @return array form
 */
function itp_payment_ajax_delete_confirm_form($form, &$form_state) {

  $idr = $form_state['build_info']['args']['idr'];

  $form['text'] = array(
    '#markup' => t('Do you want delete data?'),
    '#prefix' => '<div class="itp-payment-ajax-confirm-form"><div class="itp-payment-confirm-text">',
    '#suffix' => '</div>'
  );

  $form['idr'] = array(
    '#type' => 'hidden',
    '#value' => $idr
  );

  // Close link.
  $close = '<div class="itp-payment-close-dialog-box itp-payment-button-table">' . l(t('Cancel'), '#',
      array(
        'attributes' => array(
          'class' => array(
            'ctools-close-modal',
          ),
        ),
        'external' => TRUE
      )
    ) . '</div>';

  $form['close'] = array(
    '#markup' => $close,
  );

  $form['confirm_delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
    '#name' => 'Delete',
    '#prefix' => '<div class="itp-payment-ajax-button-table itp-payment-confirm-button">',
    '#suffix' => '</div></div>',
    '#submit' => array('itp_payment_ajax_delete_confirm_form_submit'),
  );

  return $form;
}

/**
 * Show confirm delete form.
 *
 * @param $idr integer id order in report table
 *
 * @return string
 */
function itp_payment_confirm_delete($idr, $js = NULL) {

  ctools_include('modal');
  ctools_include('ajax');

  // Fall back if $js is not set.
  if (!$js) {
    return drupal_get_form('itp_payment_ajax_delete_confirm_form');
  }

  $form_state = array(
    'title' => t('Confirm delete'),
    'ajax' => TRUE,
    'build_info' => array(
      'args' => array('idr' => $idr),
    ),
  );

  // Get form for out through drupal ajax.
  $output = ctools_modal_form_wrapper('itp_payment_ajax_delete_confirm_form', $form_state);

  // If form successfully sent.
  if (!empty($form_state['executed'])) {
    // We'll just overwrite the form output if it was successful.
    $output = array();
    ctools_add_js('ajax-responder');
    if (isset($_GET['destination'])) {
      $output[] = ctools_ajax_command_redirect($_GET['destination']);
    }
    else {
      // Closed form.
      $output[] = ctools_ajax_command_reload();
    }
  }

  // Display all in json for ajax.
  print ajax_render($output);
  exit;
}

/**
 * Delete order from Report Table.
 *
 * @param $form
 * @param $form_state
 */
function itp_payment_ajax_delete_confirm_form_submit(&$form, &$form_state) {

  // Get order ID from form data.
  $idr = $form_state['values']['idr'];

  $del = db_delete('itp_payment_report')
    ->condition('idr', $idr)
    ->execute();

  if (isset($del)) {
    drupal_set_message(t('Order deleted.'));
  }
  else {
    drupal_set_message(t('Order not deleted!'), 'error');
  }
}

/**
 * Create form with one button for change status payment order.
 *
 * @param $form
 * @param $form_state
 *
 * @return array form
 */
function itp_payment_report_change_status_form($form, &$form_state) {
  $order_id = $form_state['build_info']['args'][0];

  $form['change_' . $order_id] = array(
    '#type' => 'submit',
    '#value' => t('Change'),
    '#name' => 'Change_' . $order_id,
    '#prefix' => '<div class="itp-payment-button-table">',
    '#suffix' => '</div>',
    '#submit' => array('itp_payment_report_change_status_form_submit'),
  );
  return $form;
}

/**
 * Change status payment order from Report Table.
 *
 * @param $form
 * @param $form_state
 */
function itp_payment_report_change_status_form_submit($form, &$form_state) {
  // Get order ID from form data.
  foreach ($form_state['input'] as $key => $value) {
    $data[] = $key;
  }
  list($operation, $idr) = explode('_', $data[0]);

  // Get payment status.
  $status_query = db_select('itp_payment_report', 'r')
    ->fields('r', array('status'))
    ->condition('r.idr', $idr);
  $status = $status_query->execute()->fetchField();

  // Change status in DB.
  if ($status == 'Paid') {
    $upd = db_update('itp_payment_report')
      ->fields(array('status' => 'Not paid'))
      ->condition('idr', $idr)
      ->execute();

    if ($upd) {
      drupal_set_message(t('Payment status changed.'));
    }
    else {
      drupal_set_message(t('Payment status don\'t changed!'), 'error');
    }
  }
  else {
    $upd = db_update('itp_payment_report')
      ->fields(array('status' => 'Paid'))
      ->condition('idr', $idr)
      ->execute();

    if ($upd) {
      drupal_set_message(t('Payment status changed.'));
    }
    else {
      drupal_set_message(t('Payment status don\'t changed!'), 'error');
    }
  }
}

/**
 * Create form with one button for export users payers.
 *
 * @param $form
 * @param $form_state
 *
 * @return array form
 */
function itp_payment_report_csv_export_form($form, &$form_state) {
  $form['date_between'] = array(
    '#type' => 'item',
//    '#title' => t('Date payment'),
    'date_begin' => array(
      '#type' => 'textfield',
      '#title' => t('begin date'),
      '#description' => t('Enter begin date payment'),
      '#size' => 10,
      '#attributes' => array('class' => array('datepicker')),
      '#prefix' => '<div class="itp-payment-field">',
      '#suffix' => '</div>',
    ),
    'date_end' => array(
      '#type' => 'textfield',
      '#title' => t('end date'),
      '#description' => t('Enter end date payment'),
      '#size' => 10,
      '#attributes' => array('class' => array('datepicker')),
      '#prefix' => '<div class="itp-payment-field">',
      '#suffix' => '</div>',
    ),
    '#prefix' => '<div class="itp-payment-field-date itp-payment-field">',
    '#suffix' => '</div>',
  );

  $form['#after_build'] = array('itp_payment_datepicker');


  $form['export'] = array(
    '#type' => 'submit',
    '#value' => t('CSV Export'),
    '#submit' => array('itp_payment_report_csv_export_form_submit'),
    '#prefix' => '<div class="itp-payment-field itp-payment-submit itp-payment-button-table">',
    '#suffix' => '</div>',
  );
  return $form;
}

/**
 * Add datapicker to a Form.
 */
function itp_payment_datepicker($form, $form_state) {
  drupal_add_library('system', 'ui.datepicker');
  drupal_add_js("(function ($) { $('.datepicker').datepicker(); })(jQuery);", array(
    'type' => 'inline',
    'scope' => 'footer',
    'weight' => 5
  ));
  return $form;
}

/**
 * Submit XLS Export Button.
 *
 * @param $form
 * @param $form_state
 */
function itp_payment_report_csv_export_form_submit($form, &$form_state) {
  $date_begin = $form['date_between']['date_begin']['#value'];
  $date_end = $form['date_between']['date_end']['#value'];
  $query = db_select('itp_payment_report', 'r')
    ->fields('r');
  $query->leftJoin('itp_payment_processors_settings', 's', 's.ids=r.ids');
  $query->fields('s', array('name_pay', 'payer'));
  // If user selected date begin and date end.
  if (!empty($date_begin) && !empty($date_end)) {

    // Convert string date to Unix TimeStamp format.
    $date_begin = DateTime::createFromFormat('!m/d/Y', $date_begin);
    $date_begin_timestamp = $date_begin->format('U');
    $date_end = DateTime::createFromFormat('!m/d/Y H:i', $date_end . ' 23:59');
    $date_end_timestamp = $date_end->format('U');

    // Create condition query.
    $query->condition('r.date', array($date_begin_timestamp, $date_end_timestamp), 'BETWEEN');
  }
  $result = $query->execute();
  if ($result) {
    $csv_data[] = array(
      t('№ Payment'),
      t('ID in system'),
      t('Date'),
      t('User'),
      t('Type'),
      t('Product'),
      t('Amount'),
      t('Period'),
      t('Status'),
      t('Payment name'),
      t('Active now'),
      t('Payer')
    );

    foreach ($result as $value) {
      $user = user_load($value->uid);
      $node = node_load($value->nid);

      if (!$value->period) {
        $period = t('Unlimited');
        $active = $value->status == 'Paid' ? t('Yes') : t('No');
      }
      else {
        $period = $value->period . ' ' . t('days');
        $sec_in_day = 60 * 60 * 24;

        // Checking expired whether date payment.
        if ($value->date + $value->period * $sec_in_day > time()) {
          $active = $value->status == 'Paid' ? t('Yes') : t('No');
        }
        else {
          $active = t('No');
        }
      }
      $csv_data[] = array(
        $value->idr,
        $value->ind_system,
        format_date($value->date),
        $user->name,
        $node->type,
        $node->title,
        '$' . $value->amount,
        $period,
        $value->status,
        $value->name_pay,
        $active,
        $value->payer,
      );
    }
    itp_payment_report_download_csv_results($csv_data);
    exit();
  }
}

/**
 * Output in stream export file.
 *
 * @param $results array with export data
 */
function itp_payment_report_download_csv_results($results) {

  header('Content-Type: text/csv');
  header("Content-Transfer-Encoding: UTF-8");
  header('Content-Disposition: attachment; filename=users_payers.csv');
  header('Pragma: no-cache');
  header("Expires: 0");

  $outstream = fopen("php://output", "w");

  foreach ($results as $result) {
    fputcsv($outstream, $result, ';');
  }

  fclose($outstream);
}