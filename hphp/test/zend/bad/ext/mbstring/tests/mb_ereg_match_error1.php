<?php
/* Prototype  : bool mb_ereg_match(string $pattern, string $string [,string $option])
 * Description: Regular expression match for multibyte string 
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * Test mb_ereg_match by passing an incorrect number of arguments
 */

echo "*** Testing mb_ereg_match() : error conditions ***\n";


//Test mb_ereg_match with one more than the expected number of arguments
echo "\n-- Testing mb_ereg_match() function with more than expected no. of arguments --\n";
$pattern = b'string_val';
$string = b'string_val';
$option = 'string_val';
$extra_arg = 10;
var_dump( mb_ereg_match($pattern, $string, $option, $extra_arg) );

// Testing mb_ereg_match with one less than the expected number of arguments
echo "\n-- Testing mb_ereg_match() function with less than expected no. of arguments --\n";
$pattern = b'string_val';
var_dump( mb_ereg_match($pattern) );

// Testing mb_ereg_match with zero arguments
echo "\n-- Testing mb_ereg_match() function with zero arguments --\n";
var_dump( mb_ereg_match() );

echo "Done";
?>