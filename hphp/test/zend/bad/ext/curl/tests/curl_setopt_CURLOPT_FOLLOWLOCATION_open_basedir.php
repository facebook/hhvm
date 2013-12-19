<?php
print (ini_get("OPEN_BASEDIR"));
$ch = curl_init();
$succes = curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
curl_close($ch);
var_dump($succes);
?>