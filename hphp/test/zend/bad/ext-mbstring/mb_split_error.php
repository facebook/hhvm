<?php
/* Prototype  : proto array mb_split(string pattern, string string [, int limit])
 * Description: split multibyte string into array by regular expression 
 * Source code: ext/mbstring/php_mbregex.c
 * Alias to functions: 
 */

/*
 * test too few and too many parameters
 */

echo "*** Testing mb_split() : error conditions ***\n";


//Test mb_split with one more than the expected number of arguments
echo "\n-- Testing mb_split() function with more than expected no. of arguments --\n";
$pattern = ' ';
$string = 'a b c d e f g';
$limit = 0;
$extra_arg = 10;
var_dump( mb_split($pattern, $string, $limit, $extra_arg) );

// Testing mb_split with one less than the expected number of arguments
echo "\n-- Testing mb_split() function with less than expected no. of arguments --\n";
$pattern = 'string_val';
var_dump( mb_split($pattern) );

echo "Done";
?>