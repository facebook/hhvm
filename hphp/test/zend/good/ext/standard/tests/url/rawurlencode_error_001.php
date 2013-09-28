<?php
/* Prototype  : proto string rawurlencode(string str)
 * Description: URL-encodes string 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

// NB: basic functionality tested in tests/strings/001.phpt

echo "*** Testing rawurlencode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing rawurlencode() function with Zero arguments --\n";
var_dump( rawurlencode() );

//Test rawurlencode with one more than the expected number of arguments
echo "\n-- Testing rawurlencode() function with more than expected no. of arguments --\n";
$str = 'string_val';
$extra_arg = 10;
var_dump( rawurlencode($str, $extra_arg) );

echo "Done";
?>