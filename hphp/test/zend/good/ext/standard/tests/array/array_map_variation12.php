<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing buit-in function as callback function
 */

echo "*** Testing array_map() : built-in function ***\n";

$array1 = array(1, 2, 3);
$array2 = array(3, 4, 5);

echo "-- with built-in function 'pow' and two parameters --\n";
var_dump( array_map('pow', $array1, $array2));

echo "-- with built-in function 'pow' and one parameter --\n";
var_dump( array_map('pow', $array1));

echo "-- with language construct --\n";
var_dump( array_map('echo', $array1));

echo "Done";
?>