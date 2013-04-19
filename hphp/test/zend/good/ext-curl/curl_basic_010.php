<?php

$url = "http://www.example.org";
$ch = curl_init();
curl_setopt($ch, CURLOPT_PROXY, uniqid().":".uniqid());
curl_setopt($ch, CURLOPT_URL, $url);

curl_exec($ch);
var_dump(curl_error($ch));
var_dump(curl_errno($ch));
curl_close($ch);


?>