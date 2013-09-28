<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

echo "*** Testing array_map() : basic functionality ***\n";

function multiply($p, $q) {
  return ($p * $q);
}

function square($p) {
  return ($p * $p);
}

function concatenate($a, $b) {
  return "$a = $b";
}

// integer array
$arr1 = array(1, 2, 3);
$arr2 = array(4, 5, 6);

echo "-- With two integer array --\n";
var_dump( array_map('multiply', $arr1, $arr2) );

echo "-- With single integer array --\n";
var_dump( array_map('square', $arr1) );

// string array
$arr1 = array("one", "two");
$arr2 = array("single", "double");

echo "-- With string array --\n";
var_dump( array_map('concatenate', $arr1, $arr2) );

echo "Done";
?>