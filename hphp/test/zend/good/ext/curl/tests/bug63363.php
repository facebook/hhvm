<?php
$ch = curl_init();
var_dump(curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, false));
/* Case that should throw an error */
var_dump(curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, true));
var_dump(curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0));
var_dump(curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 1));
var_dump(curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 2));

curl_close($ch);
?>