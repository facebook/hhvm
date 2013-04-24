<?php

/* Prototype  : string bin2hex  ( string $str  )
 * Description: Convert binary data into hexadecimal representation
 * Source code: ext/standard/string.c
*/

echo "*** Testing bin2hex() : error conditions ***\n";

echo "\n-- Testing bin2hex() function with no arguments --\n";
var_dump( bin2hex() );

echo "\n-- Testing bin2hex() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( bin2hex("Hello World", $extra_arg) );

?> 
===DONE===