<?php
$to = 'example@example.com';

/* default setting */
mb_send_mail($to, mb_language(), "test");

/* English (iso-8859-1) */
if (mb_language("english")) {
	mb_internal_encoding("ISO-8859-1");
	mb_send_mail($to, "test ".mb_language(), "test");
}
?>