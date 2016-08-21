<?php

/**
 * Module for create payment via different payment processors
 * Developer: Danilevsky Kirill k.danilevsky@gmail.com
 *
 * Controller for cooperation payment processors with Drupal
 *
 */
class ItpPaymentController {

  protected $uid; // User ID.
  protected $nid; // Node ID.
  protected $ids; // Payment Id.
  protected $amount; // Amount payment.
  protected $period_active; // Period for payment.
  protected $sandbox_active; // Active for testing make payment

  /**
   * Not auto Constructor (get all data).
   *
   * @param $uid integer User ID.
   * @param $nid integer Node ID.
   */
  public function itp_constructor($uid, $nid, $ids, $amount) {
    $this->uid = $uid;
    $this->nid = $nid;
    $this->ids = $ids;
    $this->amount = $amount;
  }

  /**
   * Get all exist active payment.
   *
   * @return array with exist active payment
   */
  static function GetExistAllPaidPayment() {
    $query = db_select('itp_payment_report', 'r')
      ->fields('r')
      ->condition('r.status', 'Paid')
      ->execute();

    // If user made payment.
    if ($query) {
      $payment_exist = array();
      foreach ($query as $payment) {
        // Check period payment. If period active.
        if ($payment->period) {
          $sec_in_day = 60 * 60 * 24;
          // Checking expired whether date payment.
          if ($payment->date + $payment->period * $sec_in_day > time()) {
            $payment_exist[] = $payment;
          }
        } // If not use period.
        else {
          $payment_exist[] = $payment;
        }
      }
      return $payment_exist;
    }
    else {
      return '';
    }
  }

  /**
   * Check exist payment user's for product.
   *
   * @param $uid integer User ID.
   * @param $nid integer Node ID.
   *
   * @return bool if exist - TRUE, if no - FALSE.
   */
  static function CheckExistPaymentUser($uid, $nid) {
    $query = db_select('itp_payment_report', 'r')
      ->fields('r')
      ->condition('r.uid', $uid)
      ->condition('r.nid', $nid)
      ->condition('r.status', 'Paid')
      ->execute();

    // If user made payment.
    if ($query) {
      foreach ($query as $payment) {
        // Check period payment. If period active.
        if ($payment->period) {
          $sec_in_day = 60 * 60 * 24;
          // Checking expired whether date payment.
          if ($payment->date + $payment->period * $sec_in_day > time()) {
            return TRUE;
          }
        } // If not use period.
        else {
          return TRUE;
        }
      }
      return FALSE;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Add information about new payment to a report table.
   *
   * @return integer Order ID.
   */
  function AddUserToReportTable($days) {
    if (empty($days)) {
      $period = 0;
      $amount = $this->amount;
    }
    else {
      $period = $days;
      $amount = round($period * $this->amount, 2);
    }

    // If enable testing mode.
    if ($this->sandbox_active) {
      $status = 'Not paid (Test)';
    }
    else {
      $status = 'Not paid';
    }

    $data_report = array(
      'ids' => $this->ids,
      'ind_system' => '',
      'uid' => $this->uid,
      'nid' => $this->nid,
      'amount' => $amount,
      'period' => $period,
      'status' => $status,
      'date' => time(),
    );

    $order_id = db_insert('itp_payment_report')
      ->fields($data_report)
      ->execute();

    return $order_id;
  }

  /**
   * Change payment status in Report table.
   *
   * @param $order_id integer ID order in Reports table
   * @param $transactionId string ID transaction on the site payment processor
   * @param $status string status (Paid, Error etc.)
   */
  function ChangeStatusUserInReportTable($order_id, $transactionId, $status) {
    $upd = db_update('itp_payment_report')
      ->fields(array('status' => $status, 'ind_system' => $transactionId))
      ->condition('idr', $order_id)
      ->execute();
    // Call event after successful payment (Paid or Paid (Test)).
    $this->CallEventAfterPayment($order_id);
  }

  /**
   * Get from base thanks text.
   *
   * @return string thanks text.
   */
  function GetThanksText() {
    $thanks_text = db_select('itp_payment_thanks', 't')
      ->fields('t', array('text_thanks'))
      ->condition('t.idt', 1)
      ->execute()->fetchField();
    return token_replace($thanks_text);
  }

  /**
   * Get from base info text (Assign payment).
   *
   * @return string info text.
   */
  function GetInfoText($idi) {
    $info_text = db_select('itp_payment_info', 'i')
      ->fields('i', array('text_info'))
      ->condition('i.idi', $idi)
      ->execute()->fetchField();

    return token_replace($info_text);
  }

  /**
   * Create own hook_call_after_payment($variables)
   * Call event after successful payment (Paid or Paid (Test)).
   *
   * @param $order_id integer ID order from report table.
   */
  function CallEventAfterPayment($order_id) {

    // Get all information about order.
    $variables = db_select('itp_payment_report', 'r')
      ->fields('r')
      ->condition('r.idr', $order_id)
      ->execute()->fetchObject();

    // Make sure at least one module implements our hook.
    if (sizeof(module_implements('call_after_payment')) > 0) {
      // Call all modules that implement the hook, and let them make changes to $variables.
      $variables = module_invoke_all('call_after_payment', $variables);
    }
  }

  /**
   * Create form or button for view on the full node page.
   * (THIS METHOD MUST USE FOR ALL PAYMENT PROCESSORS)
   *
   * @return string html form or button.
   */
  function ViewProcessorForm() {

    return '';
  }

  /**
   * If user clicked payment button or form button.
   * (THIS METHOD MUST USE FOR ALL PAYMENT PROCESSORS)
   *
   * @param $method string payment method (Read value $_POST['submit_itp_payment']).
   * @param $days string with days for payment (Read value $_POST['days_itp_payment']).
   */
  function CheckSubmitButton($method, $days) {

  }

  /**
   * Get respond from Payment system.
   * (THIS METHOD MUST USE FOR ALL PAYMENT PROCESSORS)
   */
  function GetPaymentSystemRespond($data_uri) {

  }

  /**
   * Output image button with information about payment.
   *
   * @return string image button.
   */
  function InfoPaymentButton() {

    global $base_url;

    $info = l('<img src="' . $base_url . '/' . drupal_get_path('module', 'itp_payment') . '/images/info_icon_35.png' . '" alt="' . t('Info') .
      '" title="' . t('Information about payment') . '" />',
      'payment/nojs/info/' . $this->ids, array(
        'attributes' => array('class' => array('ctools-use-modal', 'ctools-modal-modal-popup-dynamic')),
        'html' => TRUE
      ));

    return $info;
  }

}