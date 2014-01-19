<?php
/* Prototype  : proto mixed parse_url(string url, [int url_component])
 * Description: Parse a URL and return its components 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

echo "*** Testing parse_url() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing parse_url() function with Zero arguments --\n";
var_dump( parse_url() );

//Test parse_url with one more than the expected number of arguments
echo "\n-- Testing parse_url() function with more than expected no. of arguments --\n";
$url = 'string_val';
$url_component = 10;
$extra_arg = 10;
var_dump( parse_url($url, $url_component, $extra_arg) );

echo "Done";
?>