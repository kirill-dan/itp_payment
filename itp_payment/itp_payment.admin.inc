<?php

/**
 * File for administer payment processors
 * Developer: Danilevsky Kirill k.danilevsky@gmail.com
 *
 */

/**
 * Scan directory in the processors Dir.
 *
 * @return array list directories
 */
function itp_payment_scan_processors_dir() {
  // Path to main payment processors directory.
  $path = drupal_get_path('module', 'itp_payment') . '/processors';
  $scan_dir = scandir($path);
  // Search all dirs.
  foreach ($scan_dir as $value) {
    if (is_dir($path . '/' . $value) && $value != '.' && $value != '..') {
      $dir[] = $value;
    }
  }
  return $dir;
}

/**
 * Check enabled whether payment processor.
 *
 * @return array fields payment processor.
 */
function itp_payment_check_enable_payment_processor($name_processor) {
  $processor = db_select('itp_payment_processors', 'p')
    ->fields('p')
    ->condition('p.name_processor', $name_processor)
    ->execute();
  if ($processor) {
    foreach ($processor as $value) {
      $payment_processor[] = $value;
    }
  }
  // If exist fields return it.
  if (isset($payment_processor)) {
    return $payment_processor;
  }
  else {
    return FALSE;
  }
}

/**
 * Check exist payments for the payment processor.
 *
 * @param $idp int ID payment processor
 *
 * @return array with payments
 */
function itp_payment_check_exist_payment_for_processor($idp) {
  $payments_from_processor = db_select('itp_payment_processors_settings', 'p')
    ->fields('p')
    ->condition('p.idp', $idp)
    ->execute();
  if ($payments_from_processor) {
    foreach ($payments_from_processor as $value) {
      $payments[] = $value;
    }
  }
  // If exist fields return it.
  if (isset($payments)) {
    return $payments;
  }
  else {
    return FALSE;
  }
}

/**
 * Get list all content type.
 *
 * @return array nodes type
 */
function itp_payment_get_nodes_type() {
  $nodes = db_select('node_type', 'n')
    ->fields('n', array('type', 'name'))
    ->execute();
  if ($nodes) {
    foreach ($nodes as $value) {
      $nodes_type[$value->type] = $value->name;
    }
  }
  return $nodes_type;
}

/**
 * Get default data from file processor.
 *
 * @param $name_processor string name dir processor
 * @param $type_data string how type data need return
 *
 * @return array with default data from file processor
 */
function itp_payment_get_default_type_payments($name_processor, $type_data) {
  $path_to_processor = 'processors/' . $name_processor . '/' . $name_processor;
  // Include install payment processor file.
  if (module_load_include('install', 'itp_payment', $path_to_processor)) {
    $func = 'itp_payment_processor_' . $name_processor . '_install';
    $data = $func();
    $default_data = $data['default_data'];
    // If in file install (settings) exist this type_data.
    if (isset($default_data[$type_data])) {
      return $default_data[$type_data];
    }
    else {
      return '';
    }
  } // If file not found show error.
  else {
    drupal_set_message(t('Install file don\'t found!'), 'error');
  }
  return '';
}

/**
 *  Create new form for payments (for horizontal tabs).
 *
 * @param $data array with data for create form
 *
 * @return array form data
 */
function itp_payment_create_new_form_payment_for_processor($data) {
  // Create different indexes for new forms and exist forms.
  if (mb_substr($data['tab_name'], 0, 7) == 'new_pay') {
    // Fields in new form will have processor ID index.
    $idp = $data['idp'];
  } // Fields in exist form will have processor ID + payment ID index.
  else {
    $idp = $data['idp'] . '_' . $data['ids'];
  }
  $horizontal_tab_name = 'horizontal_tabs_' . $data['idp'];
  $form[$data['tab_name']] = array(
    '#type' => 'fieldset',
    '#title' => $data['name_payment_tab'],
    '#group' => $horizontal_tab_name,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#prefix' => '<div id="itp-payment-fieldset">',
    '#suffix' => '</div>',
  );
  if (!empty($data['ids'])) {
    $form[$data['tab_name']]['ids'] = array(
      '#type' => 'textfield',
      '#title' => t('ID payment'),
      '#description' => t('ID payment'),
      '#default_value' => $data['ids'],
      '#size' => 8,
      '#disabled' => TRUE,
      '#prefix' => '<div class="itp-payment-field">',
      '#suffix' => '</div>',
    );
  }
  $form[$data['tab_name']]['name_pay_' . $idp] = array(
    '#type' => 'textfield',
    '#title' => t('Name payment'),
    '#description' => t('Name for tab payment'),
    '#default_value' => $data['name_payment'],
    '#size' => 30,
    '#maxlength' => 80,
    '#required' => TRUE,
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['content_type_' . $idp] = array(
    '#type' => 'select',
    '#title' => t('Content type'),
    '#options' => $data['content_type_all'],
    '#default_value' => $data['content_type'],
    '#validated' => TRUE,
    '#description' => t('Where you want get payments'),
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['payment_type_' . $idp] = array(
    '#type' => 'select',
    '#title' => t('Type payment'),
    '#options' => $data['pay_type_all'],
    '#default_value' => $data['pay_type'],
    '#validated' => TRUE,
    '#description' => t('How type payments you want use'),
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['period_' . $idp] = array(
    '#type' => 'checkbox',
    '#title' => t('Active period'),
    '#description' => t('Period for products'),
    '#default_value' => $data['period'],
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['amount_' . $idp] = array(
    '#type' => 'textfield',
    '#title' => t('Default amount'),
    '#description' => t('Amount payment in the USD'),
    '#default_value' => $data['amount'],
    '#size' => 20,
    '#maxlength' => 20,
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['payer_' . $idp] = array(
    '#type' => 'select',
    '#title' => t('Payer'),
    '#options' => $data['payer_all'],
    '#default_value' => $data['payer'],
    '#validated' => TRUE,
    '#description' => t('Who will be payer'),
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['user_' . $idp] = array(
    '#type' => 'textfield',
    '#title' => t('User'),
    '#description' => t('User name or e-mail'),
    '#default_value' => $data['user'],
    '#size' => 20,
    '#maxlength' => 80,
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['version_' . $idp] = array(
    '#type' => 'textfield',
    '#title' => t('Ver. processor'),
    '#description' => t('Version'),
    '#default_value' => $data['version'],
    '#size' => 10,
    '#maxlength' => 20,
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['password_' . $idp] = array(
    '#type' => 'textfield',
    '#title' => t('Password'),
    '#description' => t('User password'),
    '#default_value' => $data['password'],
    '#size' => 20,
    '#maxlength' => 80,
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['signature_' . $idp] = array(
    '#type' => 'textfield',
    '#title' => t('Signature (Private key)'),
    '#description' => t('Signature for payment'),
    '#default_value' => $data['signature'],
    '#size' => 30,
    '#maxlength' => 80,
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['merchant_id_' . $idp] = array(
    '#type' => 'textfield',
    '#title' => t('Merchant ID (Public key)'),
    '#description' => t('Merchant ID for payment'),
    '#default_value' => $data['merchant_id'],
    '#size' => 30,
    '#maxlength' => 80,
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['site_url_' . $idp] = array(
    '#type' => 'textfield',
    '#title' => t('Site URL'),
    '#description' => t('Link on a site payment processor'),
    '#default_value' => $data['site_url'],
    '#size' => 30,
    '#maxlength' => 80,
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['sandbox_url_' . $idp] = array(
    '#type' => 'textfield',
    '#title' => t('Site test sandbox URL'),
    '#description' => t('Link on a site payment processor for test'),
    '#default_value' => $data['sandbox_url'],
    '#size' => 30,
    '#maxlength' => 80,
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['sandbox_active_' . $idp] = array(
    '#type' => 'checkbox',
    '#title' => t('Active sandbox'),
    '#description' => t('Active sandbox for test'),
    '#default_value' => $data['sandbox_active'],
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  $form[$data['tab_name']]['active_' . $idp] = array(
    '#type' => 'checkbox',
    '#title' => t('Active payment'),
    '#description' => t('Select status on or off payment'),
    '#default_value' => $data['active'],
    '#prefix' => '<div class="itp-payment-field">',
    '#suffix' => '</div>',
  );
  // Show button add for new form.
  if (mb_substr($data['tab_name'], 0, 7) == 'new_pay') {
    $form[$data['tab_name']]['add_new_payment_' . $idp] = array(
      '#type' => 'submit',
      '#value' => t('Add new payment'),
      '#name' => $data['processor'] . ':' . $idp,
      '#submit' => array('itp_payment_admin_form_payment_settings_add_new_payment'),
      '#prefix' => '<div class="itp-payment-field">',
      '#suffix' => '</div>',
    );
  } // Show button edit and delete for form.
  else {
    $form[$data['tab_name']]['edit_payment_' . $idp] = array(
      '#type' => 'submit',
      '#value' => t('Edit payment'),
      '#name' => $data['processor'] . ':' . $idp,
      '#submit' => array('itp_payment_admin_form_payment_settings_edit_payment'),
      '#prefix' => '<div class="itp-payment-field">',
      '#suffix' => '</div>',
    );
    $form[$data['tab_name']]['delete_payment_' . $idp] = array(
      '#type' => 'submit',
      '#value' => t('Delete payment'),
      '#name' => $data['processor'] . ':' . $idp,
      '#submit' => array('itp_payment_admin_form_payment_settings_delete_payment'),
      '#prefix' => '<div class="itp-payment-field">',
      '#suffix' => '</div>',
    );
  }
  return $form;
}

/**
 * Add new payment to DB.
 *
 * @param $form
 * @param $form_state
 */
function itp_payment_admin_form_payment_settings_add_new_payment($form, &$form_state) {
  $name = $form_state['clicked_button']['#name'];
  list($processor_name, $idp) = explode(':', $name);
  $data = $form[$processor_name]['horizontal_tabs_' . $idp]['new_pay_' . $idp];
  // Get all data from array.
  $new_data_payment = array(
    'idp' => $idp,
    'name_pay' => $data['name_pay_' . $idp]['#value'],
    'content_type' => $data['content_type_' . $idp]['#value'],
    'pay_type' => $data['payment_type_' . $idp]['#value'],
    'period' => $data['period_' . $idp]['#value'],
    'amount' => $data['amount_' . $idp]['#value'],
    'payer' => $data['payer_' . $idp]['#value'],
    'user' => $data['user_' . $idp]['#value'],
    'version' => $data['version_' . $idp]['#value'],
    'password' => $data['password_' . $idp]['#value'],
    'signature' => $data['signature_' . $idp]['#value'],
    'merchant_id' => $data['merchant_id_' . $idp]['#value'],
    'site_url' => $data['site_url_' . $idp]['#value'],
    'sandbox_url' => $data['sandbox_url_' . $idp]['#value'],
    'sandbox_active' => $data['sandbox_active_' . $idp]['#value'],
    'active' => $data['active_' . $idp]['#value'],
  );
  // Insert new payment to DB.
  $id_payment = db_insert('itp_payment_processors_settings')
    ->fields($new_data_payment)
    ->execute();
  if (isset($id_payment)) {
    drupal_set_message(t('Payment created.'));
  }
  else {
    drupal_set_message(t('Payment not created!'), 'error');
  }
}

/**
 * Edit payment data.
 *
 * @param $form
 * @param $form_state
 */
function itp_payment_admin_form_payment_settings_edit_payment($form, &$form_state) {
  $name = $form_state['clicked_button']['#name'];
  list($processor_name, $idp_id) = explode(':', $name);
  list($idp, $ids) = explode('_', $idp_id);
  $data = $form[$processor_name]['horizontal_tabs_' . $idp][$processor_name . '_' . $ids];
  // Get all data from array.
  $new_data_payment = array(
    'idp' => $idp,
    'name_pay' => $data['name_pay_' . $idp . '_' . $ids]['#value'],
    'content_type' => $data['content_type_' . $idp . '_' . $ids]['#value'],
    'pay_type' => $data['payment_type_' . $idp . '_' . $ids]['#value'],
    'period' => $data['period_' . $idp . '_' . $ids]['#value'],
    'amount' => $data['amount_' . $idp . '_' . $ids]['#value'],
    'payer' => $data['payer_' . $idp . '_' . $ids]['#value'],
    'user' => $data['user_' . $idp . '_' . $ids]['#value'],
    'version' => $data['version_' . $idp . '_' . $ids]['#value'],
    'password' => $data['password_' . $idp . '_' . $ids]['#value'],
    'signature' => $data['signature_' . $idp . '_' . $ids]['#value'],
    'merchant_id' => $data['merchant_id_' . $idp . '_' . $ids]['#value'],
    'site_url' => $data['site_url_' . $idp . '_' . $ids]['#value'],
    'sandbox_url' => $data['sandbox_url_' . $idp . '_' . $ids]['#value'],
    'sandbox_active' => $data['sandbox_active_' . $idp . '_' . $ids]['#value'],
    'active' => $data['active_' . $idp . '_' . $ids]['#value'],
  );
  $upd = db_update('itp_payment_processors_settings')
    ->fields($new_data_payment)
    ->condition('ids', $ids)
    ->execute();
  if (isset($upd)) {
    drupal_set_message(t('Payment updated.'));
  }
  else {
    drupal_set_message(t('Payment not updated!'), 'error');
  }
}

/**
 * Delete payment.
 *
 * @param $form
 * @param $form_state
 */
function itp_payment_admin_form_payment_settings_delete_payment($form, &$form_state) {
  $name = $form_state['clicked_button']['#name'];
  list($processor_name, $idp_id) = explode(':', $name);
  list($idp, $ids) = explode('_', $idp_id);
  $del = db_delete('itp_payment_processors_settings')
    ->condition('ids', $ids)
    ->execute();
  if (isset($del)) {
    drupal_set_message(t('Payment deleted.'));
  }
  else {
    drupal_set_message(t('Payment not deleted!'), 'error');
  }
}

/**
 * Create form configuration (settings different payment processors).
 *
 * @return string
 */
function itp_payment_admin_form_payment_settings($form, &$form_state) {
  // Get all dir names from processors directory.
  $dirs = itp_payment_scan_processors_dir();
  if (!empty($dirs)) {
    foreach (itp_payment_scan_processors_dir() as $dir) {
      // Create vertical tabs for each payment processor.
      $form['vertical_tabs'] = array(
        '#type' => 'vertical_tabs',
      );
      $form[$dir] = array(
        '#type' => 'fieldset',
        '#title' => $dir,
        '#group' => 'vertical_tabs',
      );
      // Check enabled whether payment processor. If enabled then get data from DB.
      if (itp_payment_check_enable_payment_processor($dir)) {
        // Get fields from payment processor.
        foreach (itp_payment_check_enable_payment_processor($dir) as $field) {
          $idp = $field->idp;
          $name_processor = $field->name_processor;
          $title = $field->title;
          $controller = $field->controller;
          $callback_url = $field->callback_url;
          $callback_url_thanks = $field->callback_url_thanks;
        }
        $form[$dir]['#title'] = $name_processor . ' (' . t('Enabled') . ')';
        $form[$dir]['title_markup'] = array(
          '#markup' => '<div style="font-size: large; font-weight: bold">' . $title . '</div>',
        );
        $form[$dir]['idp'] = array(
          '#type' => 'textfield',
          '#title' => t('ID processor'),
          '#default_value' => $idp,
          '#size' => 8,
          '#disabled' => TRUE,
          '#prefix' => '<div class="itp-payment-field">',
          '#suffix' => '</div>',
        );
        $form[$dir]['name_processor'] = array(
          '#type' => 'textfield',
          '#title' => t('Name processor'),
          '#default_value' => $name_processor,
          '#size' => 12,
          '#disabled' => TRUE,
          '#prefix' => '<div class="itp-payment-field">',
          '#suffix' => '</div>',
        );
        $form[$dir]['title_processor_' . $dir] = array(
          '#type' => 'textfield',
          '#title' => t('Title processor'),
          '#default_value' => $title,
          '#size' => 30,
          '#maxlength' => 80,
          '#required' => TRUE,
          '#prefix' => '<div class="itp-payment-field">',
          '#suffix' => '</div>',
        );
        $form[$dir]['controller'] = array(
          '#type' => 'textfield',
          '#title' => t('Controller processor'),
          '#default_value' => $controller,
          '#size' => 18,
          '#disabled' => TRUE,
          '#prefix' => '<div class="itp-payment-field">',
          '#suffix' => '</div>',
        );
        $form[$dir]['callback_url'] = array(
          '#type' => 'textfield',
          '#title' => t('Callback URL'),
          '#default_value' => $callback_url,
          '#size' => 15,
          '#maxlength' => 50,
          '#disabled' => TRUE,
          '#prefix' => '<div class="itp-payment-field">',
          '#suffix' => '</div>',
        );
        $form[$dir]['callback_url_thanks'] = array(
          '#type' => 'textfield',
          '#title' => t('Callback URL Thanks'),
          '#default_value' => $callback_url_thanks,
          '#size' => 15,
          '#maxlength' => 50,
          '#disabled' => TRUE,
          '#prefix' => '<div class="itp-payment-field">',
          '#suffix' => '</div>',
        );
        $form[$dir]['save'] = array(
          '#type' => 'submit',
          '#value' => t('Save'),
          '#name' => $dir,
          '#submit' => array('itp_payment_admin_form_payment_settings_save_processor'),
          '#prefix' => '<div class="itp-payment-field">',
          '#suffix' => '</div>',
        );
        $form[$dir]['delete'] = array(
          '#type' => 'submit',
          '#value' => t('Delete'),
          '#name' => $dir,
          '#submit' => array('itp_payment_admin_form_payment_settings_delete_processor'),
        );
        // ----------------------------  Show horizontal tabs with payments ----------------- BEGIN ---
        $form[$dir]['horizontal_tabs_' . $idp] = array(
          '#type' => 'my_horizontal_tabs',
        );
        // Check exist payments for the payment processor.
        if ($payments = itp_payment_check_exist_payment_for_processor($idp)) {
          // Output all payments in own tab.
          foreach ($payments as $pay) {
            if ($pay->active) {
              $active_tab_text = ' <span class="fieldset-active">(' . t('Active payment') . ')</span>';
            }
            else {
              $active_tab_text = '';
            }
            $data_for_create_form = array(
              'tab_name' => $dir . '_' . $pay->ids,
              'name_payment_tab' => $pay->name_pay . $active_tab_text,
              'ids' => $pay->ids,
              'processor' => $dir,
              'idp' => $idp,
              'name_payment' => $pay->name_pay,
              'content_type_all' => itp_payment_get_nodes_type(),
              'content_type' => $pay->content_type,
              'pay_type_all' => itp_payment_get_default_type_payments($dir, 'pay_type'),
              'pay_type' => $pay->pay_type,
              'period' => $pay->period,
              'amount' => $pay->amount,
              'payer_all' => array('author' => 'Author', 'user' => 'User'),
              'payer' => $pay->payer,
              'user' => $pay->user,
              'version' => $pay->version,
              'password' => $pay->password,
              'signature' => $pay->signature,
              'merchant_id' => $pay->merchant_id,
              'site_url' => $pay->site_url,
              'sandbox_url' => $pay->sandbox_url,
              'sandbox_active' => $pay->sandbox_active,
              'active' => $pay->active,
            );
            // Join arrays together.
            array_push($form[$dir]['horizontal_tabs_' . $idp],
              itp_payment_create_new_form_payment_for_processor($data_for_create_form));
          }
        }
        // Show empty form for add new payment.
        $data_for_create_form = array(
          'tab_name' => 'new_pay_' . $idp,
          'name_payment_tab' => t('New Payment'),
          'ids' => '',
          'processor' => $dir,
          'idp' => $idp,
          'name_payment' => t('My payment for ') . $name_processor,
          'content_type_all' => itp_payment_get_nodes_type(),
          'content_type' => '',
          'pay_type_all' => itp_payment_get_default_type_payments($dir, 'pay_type'),
          'pay_type' => '',
          'period' => 0,
          'amount' => '',
          'payer_all' => array('author' => 'Author', 'user' => 'User'),
          'payer' => 'user',
          'user' => '',
          'version' => itp_payment_get_default_type_payments($dir, 'version'),
          'password' => '',
          'signature' => '',
          'merchant_id' => '',
          'site_url' => itp_payment_get_default_type_payments($dir, 'site_url'),
          'sandbox_url' => itp_payment_get_default_type_payments($dir, 'sandbox_url'),
          'sandbox_active' => FALSE,
          'active' => TRUE,
        );
        // Join arrays together.
        array_push($form[$dir]['horizontal_tabs_' . $idp],
          itp_payment_create_new_form_payment_for_processor($data_for_create_form));
        // Resort array for right work horizontal tabs (displace array to left).
        $array_keys = array_keys($form[$dir]['horizontal_tabs_' . $idp]);
        foreach ($array_keys as $key) {
          if (is_int($key)) {
            $data_pay = $form[$dir]['horizontal_tabs_' . $idp][$key];
            foreach ($data_pay as $key_name_pay => $value_pay) {
              $form[$dir]['horizontal_tabs_' . $idp][$key_name_pay] = $value_pay;
              unset($form[$dir]['horizontal_tabs_' . $idp][$key]);
            }
          }
        }
        //------------------------------------------------------------------------------------- END ---
      } // Show button for activate payment processor.
      else {
        $form[$dir]['enable'] = array(
          '#type' => 'submit',
          '#value' => t('Enable processor'),
          '#name' => $dir,
          '#submit' => array('itp_payment_admin_form_payment_settings_enable_processor'),
        );
      }
    }
  }
  return $form;
}

/**
 * Enable payment processor (add entry to DB).
 *
 * @param $form
 * @param $form_state
 */
function itp_payment_admin_form_payment_settings_enable_processor($form, &$form_state) {
  $name_processor = $form_state['clicked_button']['#name'];
  $path_to_processor = 'processors/' . $name_processor . '/' . $name_processor;
  // Include install payment processor file.
  if (module_load_include('install', 'itp_payment', $path_to_processor)) {
    $func = 'itp_payment_processor_' . $name_processor . '_install';
    $install_data = $func();
    // Insert new payment processor to DB.
    $id_processor = db_insert('itp_payment_processors')
      ->fields(array(
        'name_processor' => $install_data['install']['name_processor'],
        'title' => $install_data['install']['title'],
        'controller' => $install_data['install']['controller'],
        'callback_url' => $install_data['install']['callback_url'],
        'callback_url_thanks' => $install_data['install']['callback_url_thanks'],
      ))
      ->execute();
    if (isset($id_processor)) {
      drupal_set_message(t('Payment processor created.'));
    }
    else {
      drupal_set_message(t('Payment processor not created!'), 'error');
    }
  } // If file not found show error.
  else {
    drupal_set_message(t('Install file don\'t found!'), 'error');
  }
}

/**
 * Update data payment processor.
 *
 * @param $form
 * @param $form_state
 */
function itp_payment_admin_form_payment_settings_save_processor($form, &$form_state) {
  $name = $form_state['clicked_button']['#name'];
  $idp = $form[$name]['idp']['#value'];
  $title = $form[$name]['title_processor_' . $name]['#value'];
  $upd = db_update('itp_payment_processors')
    ->fields(array('title' => $title))
    ->condition('idp', $idp)
    ->execute();
  if (isset($upd)) {
    drupal_set_message(t('Payment processor updated.'));
  }
  else {
    drupal_set_message(t('Payment processor not updated!'), 'error');
  }
}

/**
 * Delete data payment processor from DB.
 *
 * @param $form
 * @param $form_state
 */
function itp_payment_admin_form_payment_settings_delete_processor($form, &$form_state) {
  $name = $form_state['clicked_button']['#name'];
  $idp = $form[$name]['idp']['#value'];
  db_delete('itp_payment_processors')
    ->condition('idp', $idp)
    ->execute();

  $del = db_delete('itp_payment_processors_settings')
    ->condition('idp', $idp)
    ->execute();
  if (isset($del)) {
    drupal_set_message(t('Payment processor deleted.'));
  }
  else {
    drupal_set_message(t('Payment processor not deleted!'), 'error');
  }
}

/**
 * Form thanks text.
 *
 * @param $form
 * @param $form_state
 *
 * @return array form
 */
function itp_payment_form_thanks_text($form, &$form_state) {
  // Get thanks text from DB.
  $thanks_text = db_select('itp_payment_thanks', 't')
    ->fields('t', array('text_thanks'))
    ->condition('idt', 1)
    ->execute()
    ->fetchField();
  $form['thanks_text'] = array(
    '#type' => 'text_format',
    '#title' => t('Thanks text'),
    '#description' => t('Enter the thanks text.'),
    '#default_value' => $thanks_text,
    '#required' => TRUE,
  );
  $form['token_help'] = array(
    '#type' => 'fieldset',
    '#title' => t('Available Tokens List'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['token_help']['content'] = array(
    '#type' => 'markup',
    '#token_types' => array(),
    '#theme' => 'token_tree',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('itp_payment_form_thanks_text_submit'),
  );
  return $form;
}

/**
 * Submit form thanks text.
 *
 * Edit thanks text in table.
 */
function itp_payment_form_thanks_text_submit($form, &$form_state) {
  // Insert to DB thanks text.
  $thanks_text = $form['thanks_text']['value']['#value'];
  $update = db_update('itp_payment_thanks')
    ->fields(array(
      'text_thanks' => $thanks_text
    ))
    ->condition('idt', 1)
    ->execute();
  if (isset($update)) {
    drupal_set_message(t('Thanks text successful updated.'));
  }
  else {
    drupal_set_message(t('Thanks text not updated!'), 'error');
  }
}

/**
 * Get admin page with Info text forms for different payment settings.
 *
 * @return array form
 */
function itp_payment_admin_page_info_text($form, $form_state) {

  // Get all payment settings.
  $payment_settings_query = db_select('itp_payment_processors_settings', 's')
    ->fields('s')->execute();

  // If exist payment settings.
  if ($payment_settings_query) {
    foreach ($payment_settings_query as $payment_settings) {
      $payment_processor = db_select('itp_payment_processors', 'p')
        ->fields('p', array('name_processor'))
        ->condition('p.idp', $payment_settings->idp)
        ->execute()->FetchField();

      // Create vertical tabs for each payment settings.
      $form['vertical_tabs'] = array(
        '#type' => 'vertical_tabs',
      );

      $form['payment_settings_' . $payment_settings->ids] = array(
        '#type' => 'fieldset',
        '#title' => $payment_settings->name_pay . ' (' . $payment_processor . ')',
        '#group' => 'vertical_tabs',
      );

      $form['payment_settings_' . $payment_settings->ids]['title_payment_settings'] = array(
        '#markup' => '<div style="font-size: large; font-weight: bold">' . $payment_settings->name_pay .
          ' (' . $payment_processor . ')' . '</div>',
      );

      // Get forms for payment setting and add to page with form.
      $form['payment_settings_' . $payment_settings->ids] += itp_payment_form_info_text($payment_settings->ids);

    }
  }

  return $form;
}

/**
 * Form info text (Assign payment).
 *
 * @param $ids integer id payment settings
 *
 * @return array form
 *
 */
function itp_payment_form_info_text($ids) {

  // Get thanks text from DB.
  $info_text = db_select('itp_payment_info', 'i')
    ->fields('i', array('idi', 'text_info'))
    ->condition('ids', $ids)
    ->execute()->fetchObject();

  // If exist ifo text.
  if ($info_text) {
    $text_info = $info_text->text_info;
    $idi = $info_text->idi;
  }
  else {
    $text_info = '';
    $idi = '';
  }

  $form['payment_settings_' . $ids]['info_text_' . $ids] = array(
    '#type' => 'text_format',
    '#title' => t('Information for user'),
    '#description' => t('Enter info text (assign payment)'),
    '#default_value' => $text_info,
  );

  $form['payment_settings_' . $ids]['idi_' . $ids] = array(
    '#type' => 'hidden',
    '#default_value' => $idi,
  );

  $form['payment_settings_' . $ids]['ids_' . $ids] = array(
    '#type' => 'hidden',
    '#default_value' => $ids,
  );

  $form['payment_settings_' . $ids]['token_help_' . $ids] = array(
    '#type' => 'fieldset',
    '#title' => t('Available Tokens List'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['payment_settings_' . $ids]['token_help']['content_' . $ids] = array(
    '#type' => 'markup',
    '#token_types' => array(),
    '#theme' => 'token_tree',
  );

  $form['payment_settings_' . $ids]['submit'] = array(
    '#type' => 'submit',
    '#name' => 'info_text:' . $ids,
    '#value' => t('Save'),
    '#submit' => array('itp_payment_form_info_text_submit'),
  );

  return $form;
}

/**
 * Submit Form info text (Assign payment).
 *
 * @param $form
 * @param $form_state
 */
function itp_payment_form_info_text_submit($form, &$form_state) {
  $ids_button = $form_state['clicked_button']['#name'];
  list($button, $ids) = explode(':', $ids_button);
  $idi = $form_state['values']['idi_' . $ids];
  $info_text = $form_state['values']['info_text_' . $ids]['value'];

  // Update or add info text for payment setting to DB.
  if ($idi) {
    db_update('itp_payment_info')
      ->fields(array('text_info' => $info_text))
      ->condition('idi', $idi)
      ->execute();
  }
  else {
    db_insert('itp_payment_info')
      ->fields(array('ids' => $ids, 'text_info' => $info_text))
      ->execute();
  }
  drupal_set_message(t('Text info updated.'));
}