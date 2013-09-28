<?php

/* Prototype  : mixed sscanf  ( string $str  , string $format  [, mixed &$...  ] )
 * Description: Parses input from a string according to a format
 * Source code: ext/standard/string.c
*/

echo "*** Testing sscanf() : basic functionality - - using hexadecimal format ***\n";

$str = "129 12F 123B -123B 01ABC 1G";
$format1 = "%x %x %x %x %x %x";
$format2 = "%X %X %X %X %X %X";

echo "\n-- Try sccanf() WITHOUT optional args --\n"; 
// extract details using short format
list($arg1, $arg2, $arg3, $arg4, $arg5, $arg6) = sscanf($str, $format1);
var_dump($arg1, $arg2, $arg3, $arg4, $arg5, $arg6);
list($arg1, $arg2, $arg3, $arg4, $arg5, $arg6) = sscanf($str, $format2);
var_dump($arg1, $arg2, $arg3, $arg4, $arg5, $arg6);

echo "\n-- Try sccanf() WITH optional args --\n"; 
// extract details using long  format
$res = sscanf($str, $format1, $arg1, $arg2, $arg3, $arg4, $arg5, $arg6);
var_dump($res, $arg1, $arg2, $arg3, $arg4, $arg5, $arg6); 
$res = sscanf($str, $format2, $arg1, $arg2, $arg3, $arg4, $arg5, $arg6);
var_dump($res, $arg1, $arg2, $arg3, $arg4, $arg5, $arg6); 

?>
===DONE===