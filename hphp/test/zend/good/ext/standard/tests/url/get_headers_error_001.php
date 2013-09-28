<?php
/* Prototype  : proto array get_headers(string url[, int format])
 * Description: Fetches all the headers sent by the server in response to a HTTP request
 * Source code: ext/standard/url.c
 * Alias to functions:
 */

echo "*** Testing get_headers() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing get_headers() function with Zero arguments --\n";
var_dump( get_headers() );

//Test get_headers with one more than the expected number of arguments
echo "\n-- Testing get_headers() function with more than expected no. of arguments --\n";
$url       = 'string_val';
$format    = 1;
$extra_arg = 10;
var_dump( get_headers($url, $format, $extra_arg) );

echo "Done";
?>