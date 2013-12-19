<?php
$to = 'example@example.com';

/* default setting */
mb_send_mail($to, mb_language(), "test");

/* Traditional Chinese () */
if (mb_language("traditional chinese")) {
	mb_internal_encoding('BIG5');
	mb_send_mail($to, "ด๚ล็ ".mb_language(), "ด๚ล็");
}
?>