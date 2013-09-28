<?php
/* Prototype  : int mb_strrpos(string $haystack, string $needle [, int $offset [, string $encoding]])
 * Description: Find position of last occurrence of a string within another 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass mb_strrpos() an incorrect number of arguments
 */

echo "*** Testing mb_strrpos() : error conditions ***\n";


//Test mb_strrpos with one more than the expected number of arguments
echo "\n-- Testing mb_strrpos() function with more than expected no. of arguments --\n";
$haystack = 'string_val';
$needle = 'string_val';
$offset = 10;
$encoding = 'string_val';
$extra_arg = 10;
var_dump( mb_strrpos($haystack, $needle, $offset, $encoding, $extra_arg) );

// Testing mb_strrpos with one less than the expected number of arguments
echo "\n-- Testing mb_strrpos() function with less than expected no. of arguments --\n";
$haystack = 'string_val';
var_dump( mb_strrpos($haystack) );

echo "Done";
?>