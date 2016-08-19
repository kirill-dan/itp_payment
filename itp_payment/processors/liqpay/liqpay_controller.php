<?php
/**
 *  Controller for communication Drupal 7 and LiqPay payment processor.
 */

/**
 *  Controller for communication Drupal 7 and LiqPal payment processor.
 */
class LiqPay_Controller extends ItpPaymentController {

  protected $name_pay; // Name payment (Propose of payment)
  protected $pay_type; // Type payment (For example: express, credit cart and etc.)
  protected $user; // User name
  protected $version; // Version payment processor
  protected $password; // Password for make payment
  protected $_public_key; // Merchant id
  protected $_private_key; // Signature

  protected $callback_url; // This site callback
  protected $callback_url_thanks; // This site callback thanks page
  protected $site_url; // Site when will redirect user for make payment
  protected $sandbox_url; // Site for testing make payment

  protected $order_id; // ID order in report table;

  /**
   * Not auto Constructor (get all data).
   * (THIS METHOD MUST USE FOR ALL PAYMENT PROCESSORS)
   *
   * @param $uid integer User ID.
   * @param $nid integer Node ID.
   * @param $payment_info array information about payment.
   */
  function constructor($uid, $nid, $payment_info) {
    parent::itp_constructor($uid, $nid, $payment_info->ids, $payment_info->amount);
    $this->name_pay = $payment_info->name_pay;
    $this->pay_type = $payment_info->pay_type;
    $this->user = $payment_info->user;
    $this->version = $payment_info->version;
    $this->password = $payment_info->password;
    $this->_public_key = $payment_info->merchant_id;
    $this->_private_key = $payment_info->signature;
    $this->callback_url = $payment_info->callback_url;
    $this->callback_url_thanks = $payment_info->callback_url_thanks;
    $this->site_url = $payment_info->site_url;
    $this->sandbox_url = $payment_info->sandbox_url;
    $this->sandbox_active = $payment_info->sandbox_active;
    $this->period_active = $payment_info->period;
  }

  /**
   * Create form or button for view on the full node page.
   * (THIS METHOD MUST USE FOR ALL PAYMENT PROCESSORS)
   *
   * @return string html form or button.
   */
  function ViewProcessorForm() {
    $payment_exist = $this->CheckExistPaymentUser($this->uid, $this->nid);
    // If user not paid for this product (nid).
    if (!$payment_exist) {
      // Create form for payment type.
      switch ($this->pay_type) {
        case 'express':
          return $this->CreateExpressForm();
          break;
      }
    }
    return '';
  }

  /**
   * Create html form for ExpressCheckout payment.
   *
   * Button Submit must have name="submit_itp_payment".
   * Hidden input must have name="itp_payment_method" and have value="METHOD"
   * Select input must have name="itp-payment-days"
   *
   */
  function CreateExpressForm() {

    global $language;

    // Show button without period select.
    if (!$this->period_active) {
      $btn_path = file_create_url(drupal_get_path('module', 'itp_payment') . '/processors/liqpay/img/btn_express_' . $language->language . '.png');
      $btn_img = '<div class="view-itp-processor-form" style="width: 250px">
                    <div class="itp-payment-name-payment">' . $this->name_pay . '</div>
                    <div class="itp-payment-info">' . $this->InfoPaymentButton() . '</div>
                    <div class="itp-payment-amount">
                         <span class="itp-payment-price-text">' . t('Price:') . '</span>
                         <span class="itp-payment-price">' . $this->amount . '</span>
                         <span class="itp-payment-price-currency">USD</span>
                    </div>
                    <form action="" method="post" name="itp_payment_form">
                      <input type="hidden" name="itp_payment_method" value="SetExpressCheckout">
                      <div class="itp-payment-button">
                        <input type="image" src="' . $btn_path . '" border="0" name="submit_itp_payment" value="SetExpressCheckout">
                      </div>
                    </form>
                  </div>';
      return $btn_img;
    } // Show button with period select.
    else {
      $btn_path = file_create_url(drupal_get_path('module', 'itp_payment') . '/processors/liqpay/img/btn_express_' . $language->language . '.png');
      $btn_img = '<div class="view-itp-processor-form" style="width: 250px">
                    <div class="itp-payment-name-payment">' . $this->name_pay . '</div>
                    <div class="itp-payment-info">' . $this->InfoPaymentButton() . '</div>
                    <div class="itp-payment-amount">
                         <span class="itp-payment-price-text">' . t('Price:') . '</span>
                         <span class="itp-payment-price">' . $this->amount . '</span>
                         <span class="itp-payment-price-currency">USD</span>
                         <span> ' . t('per day') . '</span>
                    </div>
                    <form action="" method="post" name="itp_payment_form">
                      <input type="hidden" name="itp_payment_method" value="SetExpressCheckout">
                      <span class="itp-payment-select-days">
                        <select id="itp-payment-days" name="itp-payment-days"
                        onClick=document.getElementById("itp-payment-select-days-amount").innerHTML=(this.value*' . $this->amount . ').toFixed(2)>
                            <option disabled>' . t('Select days:') . '</option>
                            <option value="7">' . t('7 days') . '</option>
                            <option value="14">' . t('14 days') . '</option>
                            <option value="30">' . t('30 days') . '</option>
                            <option value="60">' . t('60 days') . '</option>
                            <option value="120">' . t('120 days') . '</option>
                            <option value="240">' . t('240 days') . '</option>
                            <option value="360">' . t('360 days') . '</option>
                        </select>
                      </span>
                      <div class="itp-payment-amount">
                        <span class="itp-payment-price-text">' . t('Total:') . '</span>
                        <span id="itp-payment-select-days-amount" class="itp-payment-price">' . round($this->amount * 7, 2) . '</span>
                        <span class="itp-payment-price-currency">USD</span>
                      </div>
                      <div class="itp-payment-button">
                        <input type="image" src="' . $btn_path . '" border="0" name="submit_itp_payment" value="SetExpressCheckout">
                      </div>
                    </form>
                  </div>';
      return $btn_img;
    }
  }

  /**
   * If user clicked payment button or form button.
   * (THIS METHOD MUST USE FOR ALL PAYMENT PROCESSORS)
   *
   * @param $method string payment method (Read value $_POST['submit_itp_payment']).
   * @param $days string with days for payment (Read value $_POST['days_itp_payment']).
   */
  function CheckSubmitButton($method, $days) {

    global $base_url;

    // Create form for payment type.
    switch ($method) {
      case 'SetExpressCheckout':

        // Get params for payment.
        $params = $this->CreateParamsForRequest($days);
        $json = json_encode($params);
        $private_key = $this->_private_key;
        $data = base64_encode($json);
        $signature = base64_encode(sha1($private_key . $data . $private_key, 1));

        // Create form and submit form.
        print '<script>(function ($) {
             var form = $(document.createElement("form"));
              $(form).attr("action", "' . $this->site_url . '");
              $(form).attr("method", "POST");

              var input = $("<input>")
                  .attr("type", "hidden")
                  .attr("name", "data")
                  .val("' . $data . '" );

              var input1 = $("<input>")
                  .attr("type", "hidden")
                  .attr("name", "signature")
                  .val("' . $signature . '" );

              $(form).append($(input));
              $(form).append($(input1));

              form.appendTo(document.body)
              $(form).submit();
        })(jQuery);</script>';
        break;
    }
  }

  /**
   * Create Params for method LiqPayRequest.
   *
   * @param $days integer Amount days for payment.
   *
   * @return array params.
   */
  function CreateParamsForRequest($days) {
    global $base_url; // Url to our site.
    global $language;

    // Our request parameters.
    $requestParams = array(
      'version' => $this->version,
      'public_key' => $this->_public_key,
      'server_url' => $base_url . '/payment/' . $this->callback_url,
      'result_url' => $base_url . '/payment/' . $this->callback_url_thanks,
    );

    // Get sum payment.
    if (empty($days)) {
      $amount = $this->amount;
      $amount_days = 1;
    }
    else {
      $amount = round($this->amount * $days, 2);
      $amount_days = $days;
    }

    $sandbox = $this->sandbox_active ? 1 : 0;

    $orderParams = array(
      'amount' => $amount,
      'currency' => 'USD',
      'description' => $this->name_pay,
      'type' => 'buy',
      'language' => $language->language,
      'sandbox' => $sandbox
    );

    // Create new order entry in a Report Table.
    $this->order_id = $this->AddUserToReportTable($days);

    $title = node_load($this->nid)->title;
    $item = array(
      'order_id' => $this->order_id,
    );
    $params = $requestParams + $orderParams + $item;

    return $params;
  }

  /**
   * Get respond from Payment system (LiqPay).
   * (THIS METHOD MUST USE FOR ALL PAYMENT PROCESSORS)
   */
  function GetPaymentSystemRespond($data_respond) {
    // Get respond from LiqPay.
    if (isset($data_respond['data']) && isset($data_respond['signature'])) {

      $data_base = base64_decode($data_respond['data']);
      $data = json_decode($data_base, TRUE);

      if (is_array($data)) {

        // Get LiqPay transaction_id.
        $transactionId = $data['transaction_id'];
        $order_id = $data['order_id'];

        switch ($data['status']) {
          case 'sandbox':
            // Change payment status in Report table and send ID transaction.
            $this->ChangeStatusUserInReportTable($order_id, $transactionId, 'Paid (Test)');
            break;
          case 'success':
            // Change payment status in Report table and send ID transaction.
            $this->ChangeStatusUserInReportTable($order_id, $transactionId, 'Paid');
            break;
          case 'failure':
            $this->ChangeStatusUserInReportTable($order_id, $transactionId, 'Not paid (failure)');
            break;
          case 'wait_secure':
            $this->ChangeStatusUserInReportTable($order_id, $transactionId, 'Not paid (wait_secure)');
            break;
          case 'wait_accept':
            $this->ChangeStatusUserInReportTable($order_id, $transactionId, 'Not paid (wait_accept)');
            break;
          case 'wait_lc':
            $this->ChangeStatusUserInReportTable($order_id, $transactionId, 'Not paid (wait_lc)');
            break;
          case 'processing':
            $this->ChangeStatusUserInReportTable($order_id, $transactionId, 'Not paid (processing)');
            break;
          case 'cash_wait':
            $this->ChangeStatusUserInReportTable($order_id, $transactionId, 'Not paid (cash_wait)');
            break;
        }
      }
    }
  }
}