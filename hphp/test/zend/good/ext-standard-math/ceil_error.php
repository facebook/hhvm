<?php
/* Prototype  : float ceil  ( float $value  )
 * Description: Round fractions up.
 * Source code: ext/standard/math.c
 */

echo "*** Testing ceil() :  error conditions ***\n";
$arg_0 = 1.0;
$extra_arg = 1;

echo "\nToo many arguments\n";
var_dump(ceil($arg_0, $extra_arg));

echo "\nToo few arguments\n";
var_dump(ceil());
?>
===Done===