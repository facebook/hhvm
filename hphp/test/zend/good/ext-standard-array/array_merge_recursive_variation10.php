<?php
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_merge_recursive() by passing 
 * two dimensional arrays for $arr1 argument.
*/

echo "*** Testing array_merge_recursive() : two dimensional array for \$arr1 argument ***\n";

// initialize the 2-d array
$arr1 = array( 
  array(1, 2, 3, 1),
  "array" => array("hello", "world", "str1" => "hello", "str2" => 'world'),
  array(1 => "one", 2 => "two", "one", 'two'),
  array(1, 2, 3, 1)
);

// initialize the second argument 
$arr2 = array(1, "hello", "array" => array("hello", 'world'));

echo "-- Passing the entire 2-d array --\n";
echo "-- With default argument --\n";
var_dump( array_merge_recursive($arr1) );
echo "-- With more arguments --\n";
var_dump( array_merge_recursive($arr1, $arr2) );

echo "-- Passing the sub-array --\n";
echo "-- With default argument --\n";
var_dump( array_merge_recursive($arr1["array"]) );
echo "-- With more arguments --\n";
var_dump( array_merge_recursive($arr1["array"], $arr2["array"]) );

echo "Done";
?>