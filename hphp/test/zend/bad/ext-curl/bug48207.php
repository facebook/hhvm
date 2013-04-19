<?php
/*
 * Description       : Adds a file which stores the received data from curl_exec();
 * Source code       : ext/curl/multi.c
 * Test documentation: http://wiki.php.net/qa/temp/ext/curl
 */

// Figure out what handler to use
$host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');
if(!empty($host)) {

    // Use the set Environment variable
    $url = "$host/get.php?test=1";

} else {

    // Create a temporary file for the test
    $tempname = tempnam(sys_get_temp_dir(), 'CURL_HANDLE');
    $url = 'file://'. $tempname;

    // add the test data to the file
    file_put_contents($tempname, "Hello World!\nHello World!");
}


$tempfile	= tempnam(sys_get_temp_dir(), 'CURL_FILE_HANDLE');

$ch = curl_init($url);
$fp = fopen($tempfile, "r"); // Opening 'fubar' with the incorrect readonly flag
curl_setopt($ch, CURLOPT_FILE, $fp);
curl_exec($ch);
curl_close($ch);
is_file($tempfile) and @unlink($tempfile);
isset($tempname) and is_file($tempname) and @unlink($tempname);
?>