<?php

include 'server.inc';
$host = curl_cli_server_start();
$ch = curl_init();
curl_setopt($ch, CURLOPT_SAFE_UPLOAD, 1);
curl_setopt($ch, CURLOPT_URL, "{$host}/get.php?test=file");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

$file = curl_file_create(__DIR__ . '/curl_testdata1.txt');
$params = array('file' => $file);
curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
var_dump(curl_exec($ch));

$file = curl_file_create(__DIR__ . '/curl_testdata1.txt', "text/plain");
$params = array('file' => $file);
curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
var_dump(curl_exec($ch));

$file = curl_file_create(__DIR__ . '/curl_testdata1.txt', null, "foo.txt");
$params = array('file' => $file);
curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
var_dump(curl_exec($ch));

$file = curl_file_create(__DIR__ . '/curl_testdata1.txt', "text/plain", "foo.txt");
$params = array('file' => $file);
curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
var_dump(curl_exec($ch));


curl_close($ch);
?>
