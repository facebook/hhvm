<?php
$to = 'example@example.com';

/* default setting */
mb_send_mail($to, mb_language(), "test");

/* Simplified Chinese (HK-GB-2312) */
if (mb_language("simplified chinese")) {
	mb_internal_encoding('GB2312');
	mb_send_mail($to, "Втбщ ".mb_language(), "Втбщ");
}
?>