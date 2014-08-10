<?php

/* Prototype  : string quotemeta  ( string $str  )
 * Description: Quote meta characters
 * Source code: ext/standard/string.c
*/

echo "*** Testing quotemeta() : error conditions ***\n";

echo "\n-- Testing quotemeta() function with no arguments --\n";
var_dump( quotemeta());

echo "\n-- Testing quotemeta() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump(quotemeta("How are you ?", $extra_arg));

?>
===DONE===
