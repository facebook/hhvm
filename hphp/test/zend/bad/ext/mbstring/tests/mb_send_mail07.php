<?php
$to = 'example@example.com';

/* default setting */
mb_send_mail($to, mb_language(), "test");

/* Korean */
if (mb_language("korean")) {
	mb_internal_encoding('EUC-KR');
	mb_send_mail($to, "�׽�Ʈ ".mb_language(), "�׽�Ʈ");
}
?>