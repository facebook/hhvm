<?php
/* Prototype  : string chop ( string $str [, string $charlist] )
 * Description: Strip whitespace (or other characters) from the end of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing chop(): basic functionality
*/

echo "*** Testing chop() : basic functionality ***\n";

// Initialize all required variables
$str = "hello world\t\n\r\0\x0B  ";
$charlist = 'dl ';

// Calling chop() with default arguments
var_dump( chop($str) );

// Calling chop() with all arguments
var_dump( chop($str, $charlist) );

// Calling chop() with the charlist not present at the end of input string
var_dump( chop($str, '!') );

echo "Done\n";
?>