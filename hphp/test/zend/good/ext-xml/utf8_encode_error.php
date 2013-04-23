<?php
/* Prototype  : proto string utf8_encode(string data)
 * Description: Encodes an ISO-8859-1 string to UTF-8 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing utf8_encode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing utf8_encode() function with Zero arguments --\n";
var_dump( utf8_encode() );

//Test utf8_encode with one more than the expected number of arguments
echo "\n-- Testing utf8_encode() function with more than expected no. of arguments --\n";
$data = 'string_val';
$extra_arg = 10;
var_dump( utf8_encode($data, $extra_arg) );

echo "Done";
?>