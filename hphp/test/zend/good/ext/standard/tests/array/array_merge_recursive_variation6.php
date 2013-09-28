<?php
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_merge_recursive() by passing 
 * array having duplicate keys.
*/

echo "*** Testing array_merge_recursive() : array with duplicate keys for \$arr1 argument ***\n";

/* initialize the array having duplicate keys */
// array with numeric keys
$arr1_numeric_key = array( 1 => "one", 2 => "two", 2 => array(1, 2), 3 => "three", 1 => array("duplicate", 'strings'));
// array with string keys
$arr1_string_key = array("str1" => "hello", "str2" => 111, "str1" => "world", "str2" => 111.111);

// initialize the second argument
$arr2 = array("one", "str1" => "two", array("one", "two"));

echo "-- With default argument --\n";
var_dump( array_merge_recursive($arr1_numeric_key) );
var_dump( array_merge_recursive($arr1_string_key) );

echo "-- With more arguments --\n";
var_dump( array_merge_recursive($arr1_numeric_key, $arr2) );
var_dump( array_merge_recursive($arr1_string_key, $arr2) );

echo "Done";
?>