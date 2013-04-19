<?php
echo "*** Testing curl_copy_handle(): add options after copy ***\n";

// create a new cURL resource
$ch = curl_init();

// copy the handle
$ch2 = curl_copy_handle($ch);
var_dump(curl_getinfo($ch) === curl_getinfo($ch2));

// add some CURLOPT to the second handle
curl_setopt($ch2, CURLOPT_URL, 'http://www.example.com/');

var_dump(curl_getinfo($ch) === curl_getinfo($ch2));

// add same CURLOPT to the first handle
curl_setopt($ch, CURLOPT_URL, 'http://www.example.com/');
var_dump(curl_getinfo($ch) === curl_getinfo($ch2));

// change a CURLOPT in the second handle
curl_setopt($ch2, CURLOPT_URL, 'http://www.bar.com/');
var_dump(curl_getinfo($ch) === curl_getinfo($ch2));
?>
===DONE===