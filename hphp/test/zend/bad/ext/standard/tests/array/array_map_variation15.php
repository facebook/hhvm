<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing non existent function for $callback argument
 */

echo "*** Testing array_map() : non existent 'callback' function ***\n";

// arrays to be passed as arguments
$arr1 = array(1, 2);
$arr2 = array("one", "two");
$arr3 = array(1.1, 2.2);

var_dump( array_map('non_existent', $arr1, $arr2, $arr3) );

echo "Done";
?>
