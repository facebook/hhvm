<?php

/* Prototype  : string convert_cyr_string  ( string $str  , string $from  , string $to  )
 * Description: Convert from one Cyrillic character set to another
 * Source code: ext/standard/string.c
*/

$str = "hello";
$from = "k";
$to = "d";
$extra_arg = 10;

echo "*** Testing convert_cyr_string() : error conditions ***\n";

echo "\n-- Testing convert_cyr_string() function with no arguments --\n";
var_dump( convert_cyr_string() );

echo "\n-- Testing convert_cyr_string() function with no 'to' character set --\n";
var_dump( convert_cyr_string($str, $from) );

echo "\n-- Testing convert_cyr_string() function with more than expected no. of arguments --\n";
var_dump( convert_cyr_string($str, $from, $to, $extra_arg) );

echo "\n-- Testing convert_cyr_string() function with invalid 'from' character set --\n";
var_dump(bin2hex( convert_cyr_string($str, "?", $to) ));

echo "\n-- Testing convert_cyr_string() function with invalid 'to' character set --\n";
var_dump(bin2hex( convert_cyr_string($str, $from, "?")) );

echo "\n-- Testing convert_cyr_string() function with invalid 'from' and 'to' character set --\n";
var_dump(bin2hex( convert_cyr_string($str, ">", "?")) );

?> 
===DONE===