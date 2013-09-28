<?php
/* Prototype  : int mb_stripos(string haystack, string needle [, int offset [, string encoding]])
 * Description: Finds position of first occurrence of a string within another, case insensitive 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

/*
 * Test how mb_stripos behaves when passed an incorrect number of arguments
 */

echo "*** Testing mb_stripos() : error conditions ***\n";


//Test mb_stripos with one more than the expected number of arguments
echo "\n-- Testing mb_stripos() function with more than expected no. of arguments --\n";
$haystack = b'string_val';
$needle = b'string_val';
$offset = 10;
$encoding = 'string_val';
$extra_arg = 10;
var_dump( mb_stripos($haystack, $needle, $offset, $encoding, $extra_arg) );

// Testing mb_stripos with one less than the expected number of arguments
echo "\n-- Testing mb_stripos() function with less than expected no. of arguments --\n";
$haystack = b'string_val';
var_dump( mb_stripos($haystack) );

echo "Done";
?>