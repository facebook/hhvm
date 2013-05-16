<?php
/* Prototype  : int array_push(array $stack, mixed $var [, mixed $...])
 * Description: Pushes elements onto the end of the array 
 * Source code: ext/standard/array.c
 */

/*
 * Use PHP's maximum integer value as array key
 * then try and push new elements onto the array
 */

echo "*** Testing array_push() : error conditions ***\n";

$array = array(PHP_INT_MAX => 'max');

var_dump(array_push($array, 'new'));
var_dump($array);

echo "Done";
?>
