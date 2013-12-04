<?php
$to = 'example@example.com';
$headers = 'MIME-Version: 2.0';

mb_send_mail($to, mb_language(), "test", $headers);
?>