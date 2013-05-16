<?php
/* Prototype  : string mb_regex_encoding([string $encoding])
 * Description: Returns the current encoding for regex as a string. 
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * Test mb_regex_encoding with one more than expected number of arguments
 */

echo "*** Testing mb_regex_encoding() : error conditions ***\n";


echo "\n-- Testing mb_regex_encoding() function with more than expected no. of arguments --\n";
$encoding = 'string_val';
$extra_arg = 10;
var_dump( mb_regex_encoding($encoding, $extra_arg) );

echo "Done";
?>