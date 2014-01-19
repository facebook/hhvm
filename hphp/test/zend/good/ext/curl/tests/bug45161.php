<?php

// Fill memory for test
$ch = curl_init();
$fp = fopen('/dev/null', 'w');

/*
$i = $start = $end = 100000.00;
for ($i = 0; $i < 100; $i++) {
	curl_setopt($ch, CURLOPT_URL, 'http://127.0.0.1:9/');
	curl_setopt($ch, CURLOPT_FILE, $fp);
	curl_exec($ch);
}
*/

// Start actual test
$start = memory_get_usage() + 1024;
for($i = 0; $i < 1024; $i++) {
	curl_setopt($ch, CURLOPT_URL, 'http://127.0.0.1:9/');
	curl_setopt($ch, CURLOPT_FILE, $fp);
	curl_exec($ch);
}
if ($start < memory_get_usage()) {
	echo 'FAIL';
} else {
	echo 'PASS';
}
echo "\n";
fclose($fp);
unset($fp);
?>