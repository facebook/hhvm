<?php

$host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, "{$host}/get.php");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

var_dump(curl_getinfo($ch, CURLINFO_HTTP_CODE) == curl_getinfo($ch, CURLINFO_RESPONSE_CODE));

curl_exec($ch);
curl_close($ch);

?>