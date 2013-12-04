<?php
$to = 'example@example.com';

/* default setting */
mb_send_mail($to, mb_language(), "test");

/* Korean */
if (mb_language("korean")) {
	mb_internal_encoding('EUC-KR');
	mb_send_mail($to, "테스트 ".mb_language(), "테스트");
}
?>