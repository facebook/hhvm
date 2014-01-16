<?php
ini_set('precision ',  14);

/* 
 * proto float sqrt(float number)
 * Function is implemented in ext/standard/math.c
*/ 

$arg_0 = 1.0;
$extra_arg = 1;

echo "\nToo many arguments\n";
var_dump(sqrt($arg_0, $extra_arg));

echo "\nToo few arguments\n";
var_dump(sqrt());

?>