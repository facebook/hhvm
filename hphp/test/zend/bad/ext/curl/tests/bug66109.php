<?php

$host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, "{$host}/get.php?test=method");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

curl_setopt($ch, CURLOPT_CUSTOMREQUEST, 'DELETE');
var_dump(curl_exec($ch));

curl_setopt($ch, CURLOPT_CUSTOMREQUEST, NULL);
var_dump(curl_exec($ch));

curl_close($ch);

?>