<?php

$ch = curl_init("http://www.example.com/");
$fp = fopen("example_homepage.txt", "w");

curl_setopt($ch, CURLOPT_FILE, $fp);
curl_setopt($ch, CURLOPT_HEADER, 0);

// Use curl_version
print_r(curl_version());

// copy the handle
$ch2 = curl_copy_handle($ch);

curl_exec($ch);
curl_exec($ch2);

curl_close($ch);
curl_close($ch2);
fclose($fp);

// Test curl_setopt_array
// ======================
// create a new cURL resource
$ch3 = curl_init();

$params=['name'=>'John', 'surname'=>'Doe', 'age'=>36];

// set URL and other appropriate options
$options3 = array(
CURLOPT_URL => 'http://www.example.com/',
CURLOPT_POST => 0,
CURLOPT_POSTFIELDS => $params,
);

curl_setopt_array($ch3, ($options3));

fb_curl_getopt($ch3, 0);

// grab URL and pass it to the browser
curl_exec($ch3);

// close cURL resource, and free up system resources
curl_close($ch3);

HH\curl_create_pool('unittest', 1, 10);
$ch4 = HH\curl_init_pooled('unittest');
var_dump(HH\curl_list_pools());
HH\curl_destroy_pool('unittest');
var_dump(curl_getinfo($ch4));
var_dump(curl_errno($ch4));
var_dump(curl_error($ch4));

$mh = curl_multi_init();
curl_multi_add_handle($mh, $ch4);
$status = curl_multi_exec($mh, $active);
curl_multi_remove_handle($mh, $ch4);
$info = curl_multi_info_read($mh);
var_dump($info);
echo "ERROR!\n " . curl_multi_strerror($status);
echo "ERROR!\n " . curl_strerror($status);
$html = curl_multi_getcontent($ch4);
var_dump(curl_multi_select($mh));
$b = curl_multi_setopt($mh, CURLMOPT_PIPELINING, 3);
var_dump($b);
var_dump(curl_multi_close($mh));
curl_reset($ch4);
curl_close($ch4);

// Curl_share functions.
// Create cURL share handle and set it to share cookie data
$sh5 = curl_share_init();
curl_share_setopt($sh5, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);

// Initialize the first cURL handle and assign the share handle to it
$ch5 = curl_init("http://www.example.com/");
curl_setopt($ch5, CURLOPT_SHARE, $sh5);

// Execute the first cURL handle
curl_exec($ch5);

// Initialize the second cURL handle and assign the share handle to it
$ch6 = curl_init("http://www.example.com/");
var_dump(curl_setopt($ch6, CURLOPT_SHARE, $sh5));

// Execute the second cURL handle
//  all cookies from $ch1 handle are shared with $ch2 handle
curl_exec($ch6);

// Close the cURL share handle
curl_share_close($sh5);

// Close the cURL handles
curl_close($ch5);
curl_close($ch6);
?>
