<?php
/* Prototype  : string chop ( string $str [, string $charlist] )
 * Description: Strip whitespace (or other characters) from the end of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing chop() : error conditions
*/

echo "*** Testing chop() : error conditions ***\n";

// Zero argument
echo "\n-- Testing chop() function with Zero arguments --\n";
var_dump( chop() );

// More than expected number of arguments
echo "\n-- Testing chop() function with more than expected no. of arguments --\n";
$str = 'string_val ';
$charlist = 'string_val';
$extra_arg = 10;

var_dump( chop($str, $charlist, $extra_arg) );
var_dump( $str );

echo "Done\n";
?>