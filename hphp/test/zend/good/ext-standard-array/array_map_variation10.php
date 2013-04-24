<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing anoymous callback function with following variations
 */

echo "*** Testing array_map() : anonymous callback function ***\n";

$array1 = array(1, 2, 3);
$array2 = array(3, 4, 5);

echo "-- anonymous function with all parameters and body --\n";
var_dump( array_map( create_function('$a, $b', 'return array($a, $b);'), $array1, $array2));

echo "-- anonymous function with two parameters and passing one array --\n";
var_dump( array_map( create_function('$a, $b', 'return array($a, $b);'), $array1));

echo "-- anonymous function with NULL parameter --\n";
var_dump( array_map( create_function(NULL, 'return NULL;'), $array1));

echo "-- anonymous function with NULL body --\n";
var_dump( array_map( create_function('$a', NULL), $array1));

echo "-- passing NULL as 'arr1' --\n";
var_dump( array_map( create_function('$a', 'return array($a);'), NULL));

echo "Done";
?>