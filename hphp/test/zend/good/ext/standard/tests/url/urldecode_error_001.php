<?php
/* Prototype  : proto string urldecode(string str)
 * Description: Decodes URL-encoded string 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

// NB: basic functionality tested in tests/strings/001.phpt

echo "*** Testing urldecode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing urldecode() function with Zero arguments --\n";
var_dump( urldecode() );

//Test urldecode with one more than the expected number of arguments
echo "\n-- Testing urldecode() function with more than expected no. of arguments --\n";
$str = 'string_val';
$extra_arg = 10;
var_dump( urldecode($str, $extra_arg) );

echo "Done";
?>
