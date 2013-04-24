<?php

/* Prototype  : int strlen  ( string $string  )
 * Description: Get string length
 * Source code: ext/standard/string.c
*/

echo "*** Testing strlen() : unexpected number of arguments ***";


echo "\n-- Testing strlen() function with no arguments --\n";
var_dump( strlen() );

echo "\n-- Testing strlen() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( strlen("abc def", $extra_arg) );
?>
===DONE===