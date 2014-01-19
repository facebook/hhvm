<?php
/* Prototype  : string mb_substr(string $str, int $start [, int $length [, string $encoding]])
 * Description: Returns part of a string 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass incorrect number of arguments to mb_substr() to test behaviour
 */

echo "*** Testing mb_substr() : error conditions ***\n";

//Test mb_substr with one more than the expected number of arguments
echo "\n-- Testing mb_substr() function with more than expected no. of arguments --\n";
$str = 'string_val';
$start = 10;
$length = 10;
$encoding = 'string_val';
$extra_arg = 10;
var_dump( mb_substr($str, $start, $length, $encoding, $extra_arg) );

// Testing mb_substr with one less than the expected number of arguments
echo "\n-- Testing mb_substr() function with less than expected no. of arguments --\n";
$str = 'string_val';
var_dump( mb_substr($str) );

echo "Done";
?>