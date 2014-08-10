<?php

/* Prototype  : int ord  ( string $string  )
 * Description: Return ASCII value of character
 * Source code: ext/standard/string.c
*/

echo "*** Testing ord() : error conditions ***\n";

echo "\n-- Testing ord() function with no arguments --\n";
var_dump( ord() );

echo "\n-- Testing ord() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( ord(72, $extra_arg) );

?>
===DONE===
