<?php
$to = 'example@example.com';

/* default setting */
mb_send_mail($to, mb_language(), "test");

/* neutral (UTF-8) */
if (mb_language("neutral")) {
	mb_internal_encoding("none");
	mb_send_mail($to, "test ".mb_language(), "test");
}
?>