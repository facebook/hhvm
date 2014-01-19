<?php
/*
 * Prototype:     bool curl_setopt_array(resource $ch, array $options)
 * Description:   Sets multiple options for a cURL session.
 * Source:        ext/curl/interface.c
 * Documentation: http://wiki.php.net/qa/temp/ext/curl
 */

// Figure out what handler to use
$host = getenv('PHP_CURL_HTTP_REMOTE_SERVER');
if (!empty($host)) {
    // Use the set Environment variable
    $url = "{$host}/get.php?test=get";
} else {
    // Create a temporary file for the test
    $tempname = tempnam(sys_get_temp_dir(), 'CURL_HANDLE');
    $url = 'file://'. $tempname;
    // add the test data to the file
    file_put_contents($tempname, "Hello World!\nHello World!");
}

// Start the test
echo '== Starting test curl_setopt_array($ch, $options); ==' . "\n";

// curl handler
$ch = curl_init();

// options for the curl handler
$options = array (
    CURLOPT_URL => $url,
    CURLOPT_RETURNTRANSFER => 1
);

ob_start(); // start output buffering

curl_setopt_array($ch, $options);
$returnContent = curl_exec($ch);
curl_close($ch);

var_dump($returnContent);
isset($tempname) and is_file($tempname) and @unlink($tempname);

?>