<?php
/* Prototype  : proto string base64_decode(string str[, bool strict])
 * Description: Decodes string using MIME base64 algorithm 
 * Source code: ext/standard/base64.c
 * Alias to functions: 
 */

echo "*** Testing base64_decode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing base64_decode() function with Zero arguments --\n";
var_dump( base64_decode() );

//Test base64_decode with one more than the expected number of arguments
echo "\n-- Testing base64_decode() function with more than expected no. of arguments --\n";
$str = 'string_val';
$strict = true;
$extra_arg = 10;
var_dump( base64_decode($str, $strict, $extra_arg) );

echo "Done";
?>