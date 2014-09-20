<?php
$ch = curl_init();
var_dump(curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true));
curl_close($ch);
?>
