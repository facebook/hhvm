<?php
/* Prototype  : float round  ( float $val  [, int $precision  ] )
 * Description: Returns the rounded value of val  to specified precision (number of digits
 * after the decimal point)
 * Source code: ext/standard/math.c
 */
 
/*
 * Pass incorrect number of arguments to round() to test behaviour
 */
 
echo "*** Testing round() : error conditions ***\n";

echo "\n-- Wrong nmumber of arguments --\n";
var_dump(round());
var_dump(round(500, 10, true));

?>
===Done===