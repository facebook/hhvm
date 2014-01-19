<?php

$host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');

// start testing
echo "*** curl_setopt() call with CURLOPT_HTTPHEADER\n";

$url = "{$host}/";
$ch = curl_init();

curl_setopt($ch, CURLOPT_HTTPHEADER, 1);

$curl_content = curl_exec($ch);
curl_close($ch);

var_dump( $curl_content );

$ch = curl_init();

ob_start(); // start output buffering
curl_setopt($ch, CURLOPT_HTTPHEADER, array());
curl_setopt($ch, CURLOPT_URL, $host);

$curl_content = curl_exec($ch);
ob_end_clean();
curl_close($ch);

var_dump( $curl_content );
?>