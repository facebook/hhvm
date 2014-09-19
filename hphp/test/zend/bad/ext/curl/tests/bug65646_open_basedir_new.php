<?php
$ch = curl_init();
var_dump(curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true));
var_dump(curl_setopt($ch, CURLOPT_PROTOCOLS, CURLPROTO_FILE));
var_dump(curl_setopt($ch, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_FILE));
curl_close($ch);
?>
