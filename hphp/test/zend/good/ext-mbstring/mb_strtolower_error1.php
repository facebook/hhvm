<?php
/* Prototype  : string mb_strtolower(string $sourcestring [, string $encoding])
 * Description: Returns a lowercased version of $sourcestring
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass an incorrect number of arguments to mb_strtolower() to test behaviour
 */

echo "*** Testing mb_strtolower() : error conditions***\n";

//Test mb_strtolower with one more than the expected number of arguments
echo "\n-- Testing mb_strtolower() function with more than expected no. of arguments --\n";
$sourcestring = 'string_value';
$encoding = 'UTF-8';
$extra_arg = 10;
var_dump( mb_strtolower($sourcestring, $encoding, $extra_arg) );

//Test mb_strtolower with zero arguments
echo "\n-- Testing mb_strtolower() function with zero arguments --\n";
var_dump( mb_strtolower() );

echo "Done";
?>