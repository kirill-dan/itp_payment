<?php

/**
 *  Controller for communication Drupal 7 and PayPal payment processor.
 */
class PayPal_Controller extends ItpPaymentController {

  protected $name_pay; // Name payment (Propose of payment)
  protected $pay_type; // Type payment (For example: express, credit cart and etc.)
  protected $user; // User name
  protected $version; // Version payment processor
  protected $password; // Password for make payment
  protected $signature; // Signature
  protected $merchant_id;

  protected $callback_url; // This site callback
  protected $callback_url_thanks; // This site callback thanks page
  protected $site_url; // Site when will redirect user for make payment
  protected $sandbox_url; // Site for testing make payment

  protected $order_id; // ID order in report table;

  /**
   * Last error message(s)
   * @var array
   */
  protected $_errors = array();

  /**
   * API Credentials
   * Use the correct credentials for the environment in use (Live / Sandbox)
   * @var array
   */
  protected $credentials;

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
    $this->credentials = array(
      'USER' => $payment_info->user,
      'PWD' => $payment_info->password,
      'SIGNATURE' => $payment_info->signature,
    );
    $this->name_pay = $payment_info->name_pay;
    $this->pay_type = $payment_info->pay_type;
    $this->user = $payment_info->user;
    $this->version = $payment_info->version;
    $this->password = $payment_info->password;
    $this->signature = $payment_info->signature;
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

    // Show button without period select.
    if (!$this->period_active) {
      $btn_path = file_create_url(drupal_get_path('module', 'itp_payment') . '/processors/paypal/img/btn_express.gif');
      $btn_img = '<div class="view-itp-processor-form" style="width: 160px">
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
      $btn_path = file_create_url(drupal_get_path('module', 'itp_payment') . '/processors/paypal/img/btn_express.gif');
      $btn_img = '<div class="view-itp-processor-form" style="width: 195px">
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
    // Create form for payment type.
    switch ($method) {
      case 'SetExpressCheckout':
        // Get params for payment.
        $params = $this->CreateParamsForRequest($days);
        // Get token from PayPal.
        $response = $this->PayPalRequest($method, $params);

        if (is_array($response) && $response['ACK'] == 'Success') { // Request successful.
          $token = $response['TOKEN'];

          // Save token and payment data in session.
          $_SESSION['token'][$token]['order_id'] = $this->order_id;
          $_SESSION['token'][$token]['version'] = $this->version;
          $_SESSION['token'][$token]['credentials'] = $this->credentials;
          $_SESSION['token'][$token]['callback_url'] = $this->callback_url;
          $_SESSION['token'][$token]['callback_url_thanks'] = $this->callback_url_thanks;
          $_SESSION['token'][$token]['sandbox_active'] = $this->sandbox_active;
          $_SESSION['token'][$token]['sandbox_url'] = $this->sandbox_url;
          $_SESSION['token'][$token]['site_url'] = $this->site_url;

          // If testing mode.
          if ($this->sandbox_active) {
            header('Location: https://www.sandbox.paypal.com/webscr?cmd=_express-checkout&token=' . urlencode($token));
          }
          else {
            header('Location: https://www.paypal.com/webscr?cmd=_express-checkout&token=' . urlencode($token));
          }
        }
        else {
          drupal_set_message(t('Error payment. Please contact Administration.'), 'error');
          drupal_set_message($response['L_SHORTMESSAGE0'], 'error');
          drupal_set_message($response['L_LONGMESSAGE0'], 'error');
        }
        break;
    }
  }

  /**
   * Make API request to PayPal.
   *
   * @param string $method string API method to request
   * @param array $params Additional request parameters
   *
   * @return array / boolean Response array / boolean false on failure
   */
  public function PayPalRequest($method, $params = array()) {
    $this->_errors = array();
    // Check if API method is not empty.
    if (empty($method)) {
      $this->_errors = array('API method is missing');
      return FALSE;
    }

    // Our request parameters.
    $requestParams = array(
        'METHOD' => $method,
        'VERSION' => $this->version
      ) + $this->credentials;

    // Building our NVP string.
    $request = http_build_query($requestParams + $params);

    // Check test mode. If test mode active.
    if ($this->sandbox_active) {
      $site_url = $this->sandbox_url;
    } // If test mode not active.
    else {
      $site_url = $this->site_url;
    }

    // cURL settings.
    $curlOptions = array(
      CURLOPT_URL => $site_url,
      CURLOPT_VERBOSE => 1,
      /*      CURLOPT_SSL_VERIFYPEER => TRUE,
            CURLOPT_SSL_VERIFYHOST => 2,
            CURLOPT_CAINFO => dirname(__FILE__) . '/cacert.pem', // CA cert file.*/
      CURLOPT_RETURNTRANSFER => 1,
      CURLOPT_POST => 1,
      CURLOPT_POSTFIELDS => $request
    );

    $ch = curl_init();
    curl_setopt_array($ch, $curlOptions);

    // Sending our request - $response will hold the API response.
    $response = curl_exec($ch);

    // Checking for cURL errors.
    if (curl_errno($ch)) {
      $this->_errors = curl_error($ch);
      curl_close($ch);
      return FALSE;
      // Handle errors.
    }
    else {
      curl_close($ch);
      $responseArray = array();
      parse_str($response, $responseArray); // Break the NVP string to an array.
      return $responseArray;
    }
  }

  /**
   * Create Params for method PayPalRequest.
   *
   * @param $days integer Amount days for payment.
   *
   * @return array params.
   */
  function CreateParamsForRequest($days) {
    global $base_url; // Url to our site.

    // Our request parameters.
    $requestParams = array(
      'RETURNURL' => $base_url . '/payment/' . $this->callback_url,
      'CANCELURL' => $base_url . '/node/' . $this->nid,
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

    $orderParams = array(
      "PAYMENTREQUEST_0_PAYMENTACTION" => 'Sale',
      'PAYMENTREQUEST_0_AMT' => $amount,
      'PAYMENTREQUEST_0_CURRENCYCODE' => 'USD',
      'PAYMENTREQUEST_0_ITEMAMT' => $amount,
      'SOLUTIONTYPE' => 'Sole',
      'LANDINGPAGE' => 'Billing',
      'LOCALECODE' => 'US'
    );

    // Create new order entry in a Report Table.
    $this->order_id = $this->AddUserToReportTable($days);

    $title = node_load($this->nid)->title;
    $item = array(
      'L_PAYMENTREQUEST_0_NAME0' => $title,
      'L_PAYMENTREQUEST_0_AMT0' => $this->amount,
      'L_PAYMENTREQUEST_0_QTY0' => $amount_days,
      'PAYMENTREQUEST_0_INVNUM' => $this->order_id,
    );
    $params = $requestParams + $orderParams + $item;
    return $params;
  }

  /**
   * Get respond from Payment system (PayPal).
   * (THIS METHOD MUST USE FOR ALL PAYMENT PROCESSORS)
   */
  function GetPaymentSystemRespond($data_uri) {
    // Get token and Payer ID.
    if (!empty($data_uri)) {
      list($token_all, $payer_all) = explode('&', $data_uri);
      $token = str_replace('token=', '', $token_all);
      $payer_id = str_replace('PayerID=', '', $payer_all);
      if (!empty($token) && !empty($payer_id)) {
        if (isset($_SESSION['token'][$token])) {
          $this->credentials = $_SESSION['token'][$token]['credentials'];
          $this->version = $_SESSION['token'][$token]['version'];
          $this->callback_url = $_SESSION['token'][$token]['callback_url'];
          $this->callback_url_thanks = $_SESSION['token'][$token]['callback_url_thanks'];
          $this->site_url = $_SESSION['token'][$token]['site_url'];
          $this->sandbox_url = $_SESSION['token'][$token]['sandbox_url'];
          $this->sandbox_active = $_SESSION['token'][$token]['sandbox_active'];
          $this->order_id = $_SESSION['token'][$token]['order_id'];

          $checkoutDetails = $this->PayPalRequest('GetExpressCheckoutDetails', array('TOKEN' => $token));

          if ($checkoutDetails['INVNUM'] == $_SESSION['token'][$token]['order_id']) {
            $order_id = $checkoutDetails['INVNUM'];

            // Complete the checkout transaction.
            $requestParams = array(
              'PAYMENTREQUEST_0_PAYMENTACTION' => 'Sale',
              'PAYERID' => $payer_id,
              'TOKEN' => $token,
              'PAYMENTREQUEST_0_AMT' => $checkoutDetails['AMT'],
            );

            $response = $this->PayPalRequest('DoExpressCheckoutPayment', $requestParams);

            if (is_array($response) && $response['ACK'] == 'Success') { // Payment successful
              // We'll fetch the transaction ID for internal bookkeeping
              $transactionId = $response['PAYMENTINFO_0_TRANSACTIONID'];

              // If this is test payment.
              if ($this->sandbox_active) {
                // Change payment status in Report table and send ID transaction.
                $this->ChangeStatusUserInReportTable($order_id, $transactionId, 'Paid (Test)');
              } // If this is real payment.
              else {
                // Change payment status in Report table and send ID transaction.
                $this->ChangeStatusUserInReportTable($order_id, $transactionId, 'Paid');
              }
              // Delete paid token and date from session.
              unset($_SESSION['token'][$token]);

              // Send user on a thanks page.
              drupal_goto('payment/' . $this->callback_url_thanks);
            }
          }
        }
      }
    }
  }
}