<?php

$host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, "{$host}/get.php?test=file");
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

$params = array('file' => '@' . __DIR__ . '/curl_testdata1.txt');
curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
var_dump(curl_exec($ch));

$params = array('file' => '@' . __DIR__ . '/curl_testdata1.txt;type=text/plain');
curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
var_dump(curl_exec($ch));

$params = array('file' => '@' . __DIR__ . '/curl_testdata1.txt;filename=foo.txt');
curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
var_dump(curl_exec($ch));

$params = array('file' => '@' . __DIR__ . '/curl_testdata1.txt;type=text/plain;filename=foo.txt');
curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
var_dump(curl_exec($ch));

$params = array('file' => '@' . __DIR__ . '/curl_testdata1.txt;filename=foo.txt;type=text/plain');
curl_setopt($ch, CURLOPT_POSTFIELDS, $params);
var_dump(curl_exec($ch));


curl_close($ch);
?>