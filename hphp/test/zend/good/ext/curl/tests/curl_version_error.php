<?php

/* Prototype  : array curl_version  ([ int $age  ] )
 * Description: Returns information about the cURL version.
 * Source code: ext/curl/interface.c
*/

echo "*** Testing curl_version() : error conditions ***\n";

echo "\n-- Testing curl_version() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( curl_version(1, $extra_arg) );

?>
===Done===