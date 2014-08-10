<?php

/* Prototype  : array explode  ( string $delimiter  , string $string  [, int $limit  ] )
 * Description: Split a string by string.
 * Source code: ext/standard/string.c
*/

echo "*** Testing explode() : error conditions ***\n";

echo "\n-- Testing explode() function with no arguments --\n";
var_dump( explode() );

echo "\n-- Testing explode() function with more than expected no. of arguments --\n";
$delimiter = " ";
$string = "piece1 piece2 piece3 piece4 piece5 piece6";
$limit = 5;
$extra_arg = 10;
var_dump( explode($delimiter, $string, $limit, $extra_arg) );

?>
===Done===
