<?php
$to = 'example@example.com';

/* default setting */
mb_send_mail($to, mb_language(), "test");

/* German (iso-8859-15) */
if (mb_language("german")) {
	mb_internal_encoding("ISO-8859-15");
	mb_send_mail($to, "Pr"."\xfc"."fung ".mb_language(), "Pr"."\xfc"."fung");
}
?>