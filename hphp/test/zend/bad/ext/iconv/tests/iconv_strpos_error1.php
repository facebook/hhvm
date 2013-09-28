<?php
/* Prototype  : int iconv_strpos(string haystack, string needle [, int offset [, string charset]])
 * Description: Find position of first occurrence of a string within another 
 * Source code: ext/iconv/iconv.c
 */

/*
 * Test how iconv_strpos behaves when passed an incorrect number of arguments
 */

echo "*** Testing iconv_strpos() : error conditions ***\n";


//Test iconv_strpos with one more than the expected number of arguments
echo "\n-- Testing iconv_strpos() function with more than expected no. of arguments --\n";
$haystack = 'string_val';
$needle = 'string_val';
$offset = 10;
$encoding = 'string_val';
$extra_arg = 10;
var_dump( iconv_strpos($haystack, $needle, $offset, $encoding, $extra_arg) );

// Testing iconv_strpos with one less than the expected number of arguments
echo "\n-- Testing iconv_strpos() function with less than expected no. of arguments --\n";
$haystack = 'string_val';
var_dump( iconv_strpos($haystack) );

echo "Done";
?>