Module for create payment via different payment processors
Developer: Danilevsky Kirill k.danilevsky@gmail.com
web: http://best-house.org

This module gives you opportunity include any payment systems which there in world. Now in module included two payment systems:
PayPal and LiqPay.

For add new payment processor you have to create new folder in processors/ folder with payment name. And create two files:
payment.install and payment_controller.php with own data. See live examples and read comments.

For check exist pay use static method: ItpPaymentController::CheckExistPaymentUser($uid, $nid)
where $uid - user uid which checking and $nid - node nid which checking

For get all active pay use static method: ItpPaymentController::GetExistAllPaidPayment()

You can use hook_after_payment($variables) for Call event after successful payment (Paid or Paid (Test))

All settings you can see on path admin/itp-payment.

In "Settings" you can activate payment processor and create new payment. You can create payment only for content type. Fill need fields
and save form.

If you checked Active sanbox - payment will be create only for test.

If you select payer Author - payment form will be show only author node. Use module itp_vip_blocks for automatically advertisement.

If you select payer User - payment form will be show all authenticated users.

Use in your module or node template method ItpPaymentController::CheckExistPaymentUser($uid, $nid) for check exist active payment.

If will be checked Period - on the form will show select list with days and field amount.

In settings "Information text" will create details of payment for each payment.

In settings "Thanks text" will create text which will show user after payment.

All reports you can see in "Report payments".