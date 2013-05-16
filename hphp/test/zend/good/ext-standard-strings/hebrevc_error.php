<?php

/* Prototype  : string hebrevc  ( string $hebrew_text  [, int $max_chars_per_line  ] )
 * Description: Convert logical Hebrew text to visual text
 * Source code: ext/standard/string.c
*/

echo "*** Testing hebrevc() : error conditions ***\n";

echo "\n-- Testing hebrevc() function with no arguments --\n";
var_dump( hebrevc() );

echo "\n-- Testing hebrevc() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( hebrevc("Hello World", 5, $extra_arg) );

?> 
===DONE===