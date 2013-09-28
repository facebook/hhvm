<?php

/* Prototype  : string trim  ( string $str  [, string $charlist  ] )
 * Description: Strip whitespace (or other characters) from the beginning and end of a string.
 * Source code: ext/standard/string.c
*/


echo "*** Testing trim() : error conditions ***\n";

echo "\n-- Testing trim() function with no arguments --\n";
var_dump( trim() );

echo "\n-- Testing trim() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( trim("Hello World",  "Heo", $extra_arg) );


$hello = "  Hello World\n";
echo "\n-- Test trim function with various invalid charlists --\n";
var_dump(trim($hello, "..a"));
var_dump(trim($hello, "a.."));
var_dump(trim($hello, "z..a"));
var_dump(trim($hello, "a..b..c"));

?>
===DONE===