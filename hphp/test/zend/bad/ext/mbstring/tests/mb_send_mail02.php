<?php
$to = 'example@example.com';

/* default setting */
mb_send_mail($to, mb_language(), "test");

/* Japanese (EUC-JP) */
if (mb_language("japanese")) {
	mb_internal_encoding('EUC-JP');
	mb_send_mail($to, "テスト ".mb_language(), "テスト");
}
?>