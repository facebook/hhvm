<?php
/* Prototype  : string mb_decode_mimeheader(string string)
 * Description: Decodes the MIME "encoded-word" in the string 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_decode_mimeheader() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing mb_decode_mimeheader() function with Zero arguments --\n";
var_dump( mb_decode_mimeheader() );

//Test mb_decode_mimeheader with one more than the expected number of arguments
echo "\n-- Testing mb_decode_mimeheader() function with more than expected no. of arguments --\n";
$string = 'string_val';
$extra_arg = 10;
var_dump( mb_decode_mimeheader($string, $extra_arg) );

?>
===DONE===