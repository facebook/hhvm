<?php

/* Prototype  : mixed count_chars  ( string $string  [, int $mode  ] )
 * Description: Return information about characters used in a string
 * Source code: ext/standard/string.c
*/

echo "*** Testing count_chars() : error conditions ***\n";

echo "\n-- Testing count_chars() function with no arguments --\n";
var_dump( count_chars() );

echo "\n-- Testing count_chars() function with more than expected no. of arguments --\n";
$string = "Hello World\n"; 
$mode = 1;
$extra_arg = 10;
var_dump( count_chars($string, $mode, $extra_arg) );

?>
===DONE===