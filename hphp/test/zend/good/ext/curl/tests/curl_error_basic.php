<?php
/*
 * Prototype:     string curl_error(resource $ch)
 * Description:   Returns a clear text error message for the last cURL operation.
 * Source:        ext/curl/interface.c
 * Documentation: http://wiki.php.net/qa/temp/ext/curl
 */
 
// Fake URL to trigger an error
$url = "fakeURL";

echo "== Testing curl_error with a fake URL ==\n";

// cURL handler
$ch = curl_init($url);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);

curl_exec($ch);
var_dump(curl_error($ch));
curl_close($ch);

?>