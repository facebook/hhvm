<?php

$log_string = 'hello world';
$filename = tempnam(null, 'errorlog_test');

error_log($log_string, 3, $filename);
$f = fopen($filename, 'r');
$content = fgets($f);
var_dump($content);
fclose($f);

// test that the logging is appending without newlines
error_log($log_string, 3, $filename);
$f = fopen($filename, 'r');
$content = fgets($f);
var_dump($content);
fclose($f);

unlink($filename);
