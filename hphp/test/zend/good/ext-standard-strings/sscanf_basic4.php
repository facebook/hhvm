<?php

/* Prototype  : mixed sscanf  ( string $str  , string $format  [, mixed &$...  ] )
 * Description: Parses input from a string according to a format
 * Source code: ext/standard/string.c
*/

echo "*** Testing sscanf() : basic functionality - using char format ***\n";

$str = "X = A + B - C";
$format = "%c = %c + %c - %c";

echo "\n-- Try sccanf() WITHOUT optional args --\n"; 
// extract details using short format
list($arg1, $arg2, $arg3, $arg4) = sscanf($str, $format);
var_dump($arg1, $arg2, $arg3, $arg4);

echo "\n-- Try sccanf() WITH optional args --\n"; 
// extract details using long  format
$res = sscanf($str, $format, $arg1, $arg2, $arg3, $arg4);
var_dump($res, $arg1, $arg2, $arg3, $arg4); 

?>
===DONE===