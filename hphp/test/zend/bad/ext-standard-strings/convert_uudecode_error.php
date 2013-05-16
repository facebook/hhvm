<?php

/* Prototype  : string convert_uudecode  ( string $data  )
 * Description: Decode a uuencoded string
 * Source code: ext/standard/uuencode.c
*/

echo "*** Testing convert_uudecode() : error conditions ***\n";

echo "\n-- Testing convert_uudecode() function with no arguments --\n";
var_dump( convert_uudecode() );

echo "\n-- Testing convert_uudecode() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( convert_uudecode(72, $extra_arg) );

?> 
===DONE===