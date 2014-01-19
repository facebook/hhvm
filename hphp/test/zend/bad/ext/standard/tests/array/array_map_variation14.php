<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing null values for $callback argument and testing whether shortest
 * array will be extended with empty elements
 */

echo "*** Testing array_map() : null value for 'callback' argument ***\n";

// arrays to be passed as arguments
$arr1 = array(1, 2);
$arr2 = array("one", "two");
$arr3 = array(1.1, 2.2);

// get an unset variable
$unset_var = 10;
unset ($unset_var);

/* calling array_map() with null callback */

echo "-- with null --\n";
var_dump( array_map(null, $arr1, $arr2, $arr3) );
var_dump( array_map(NULL, $arr1, $arr2, $arr3) );

echo "-- with unset variable --\n";
var_dump( array_map(@$unset_var, $arr1, $arr2, $arr3) );

echo "-- with undefined variable --\n";
var_dump( array_map(@$undefined_var, $arr1) );

echo "-- with empty string --\n";
var_dump( array_map("", $arr1, $arr2) );

echo "-- with empty array --\n";
var_dump( array_map(array(), $arr1, $arr2) );

echo "Done";
?>