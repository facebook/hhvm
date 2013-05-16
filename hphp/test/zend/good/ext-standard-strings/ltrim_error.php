<?php

/* Prototype  : string ltrim  ( string $str  [, string $charlist  ] )
 * Description: Strip whitespace (or other characters) from the beginning of a string.
 * Source code: ext/standard/string.c
*/


echo "*** Testing ltrim() : error conditions ***\n";

echo "\n-- Testing ltrim() function with no arguments --\n";
var_dump( ltrim() );

echo "\n-- Testing ltrim() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( ltrim("Hello World",  "Heo", $extra_arg) );


$hello = "  Hello World\n";
echo "\n-- Test ltrim function with various invalid charlists\n";
var_dump(ltrim($hello, "..a"));
var_dump(ltrim($hello, "a.."));
var_dump(ltrim($hello, "z..a"));
var_dump(ltrim($hello, "a..b..c"));

?>
===DONE===