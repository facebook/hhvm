<?php

/* Prototype  : string chr  ( int $ascii  )
 * Description: Return a specific character
 * Source code: ext/standard/string.c
*/

echo "*** Testing chr() : error conditions ***\n";

echo "\n-- Testing chr() function with no arguments --\n";
var_dump( chr() );

echo "\n-- Testing chr() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( chr(72, $extra_arg) );

?>
===DONE===