<?php
/* Prototype  : int mb_strlen(string $str [, string $encoding])
 * Description: Get character numbers of a string 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass mb_strlen an incorrect number of arguments to test behaviour
 */

echo "*** Testing mb_strlen() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing mb_strlen() function with Zero arguments --\n";
var_dump( mb_strlen() );

//Test mb_strlen with one more than the expected number of arguments
echo "\n-- Testing mb_strlen() function with more than expected no. of arguments --\n";
$str = 'string_val';
$encoding = 'string_val';
$extra_arg = 10;
var_dump( mb_strlen($str, $encoding, $extra_arg) );

echo "Done";
?>