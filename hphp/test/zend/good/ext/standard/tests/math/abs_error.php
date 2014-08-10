<?php
/* Prototype  : number abs  ( mixed $number  )
 * Description: Returns the absolute value of number.
 * Source code: ext/standard/math.c
 */

/*
 * Pass incorrect number of arguments to abs() to test behaviour
 */
 
echo "*** Testing abs() : error conditions ***\n";

$arg_0 = 1.0;
$extra_arg = 1;

echo "\nToo many arguments\n";
var_dump(abs($arg_0, $extra_arg));

echo "\nToo few arguments\n";
var_dump(abs());

?>
===Done===
