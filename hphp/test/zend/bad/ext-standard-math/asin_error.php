<?php
/* 
 * proto float asin(float number)
 * Function is implemented in ext/standard/math.c
*/ 

$arg_0 = 1.0;
$extra_arg = 1;

echo "\nToo many arguments\n";
var_dump(asin($arg_0, $extra_arg));

echo "\nToo few arguments\n";
var_dump(asin());

?>