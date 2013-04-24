<?php
/* Prototype  : proto string rawurldecode(string str)
 * Description: Decodes URL-encodes string 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

// NB: basic functionality tested in tests/strings/001.phpt

echo "*** Testing rawurldecode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing rawurldecode() function with Zero arguments --\n";
var_dump( rawurldecode() );

//Test rawurldecode with one more than the expected number of arguments
echo "\n-- Testing rawurldecode() function with more than expected no. of arguments --\n";
$str = 'string_val';
$extra_arg = 10;
var_dump( rawurldecode($str, $extra_arg) );

echo "Done";
?>