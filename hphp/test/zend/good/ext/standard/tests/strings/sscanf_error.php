<?php

/* Prototype  : mixed sscanf  ( string $str  , string $format  [, mixed &$...  ] )
 * Description: Parses input from a string according to a format
 * Source code: ext/standard/string.c
*/
echo "*** Testing sscanf() : error conditions ***\n";

$str = "Hello World";
$format = "%s %s";

echo "\n-- Testing sscanf() function with no arguments --\n";
var_dump( sscanf() );

echo "\n-- Testing sscanf() function with one argument --\n";
var_dump( sscanf($str) );

echo "\n-- Testing sscanf() function with more than expected no. of arguments --\n";

var_dump( sscanf($str, $format, $str1, $str2, $extra_str) );

?>
===DONE===