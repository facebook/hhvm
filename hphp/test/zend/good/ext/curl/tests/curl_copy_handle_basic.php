<?php
echo "*** Testing curl_copy_handle(): basic ***\n";

// create a new cURL resource
$ch = curl_init();

// set URL and other appropriate options
curl_setopt($ch, CURLOPT_URL, 'http://www.example.com/');
curl_setopt($ch, CURLOPT_HEADER, 0);

// copy the handle
$ch2 = curl_copy_handle($ch);

var_dump(curl_getinfo($ch) === curl_getinfo($ch2));
?>
===DONE===