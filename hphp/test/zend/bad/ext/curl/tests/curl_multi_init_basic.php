<?php
/* Prototype         : resource curl_multi_init(void)
 * Description       : Returns a new cURL multi handle
 * Source code       : ext/curl/multi.c
 * Test documentation:  http://wiki.php.net/qa/temp/ext/curl
 */

// start testing
echo "*** Testing curl_multi_init(void); ***\n";

//create the multiple cURL handle
$mh = curl_multi_init();
var_dump($mh);

curl_multi_close($mh);
var_dump($mh);
?>
===DONE===