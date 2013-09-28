<?php
/* Prototype  : string md5  ( string $str  [, bool $raw_output= false  ] )
 * Description: Calculate the md5 hash of a string
 * Source code: ext/standard/md5.c
*/

echo "*** Testing md5() : error conditions ***\n";

echo "\n-- Testing md5() function with no arguments --\n";
var_dump( md5());

echo "\n-- Testing md5() function with more than expected no. of arguments --\n";
$str = "Hello World";
$raw_output = true;
$extra_arg = 10;

var_dump(md5($str, $raw_output, $extra_arg));
?>
===DONE==