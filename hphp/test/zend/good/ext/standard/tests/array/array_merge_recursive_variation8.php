<?php
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_merge_recursive() by passing an array having binary values.
*/

echo "*** Testing array_merge_recursive() : array with binary data for \$arr1 argument ***\n";

// array with binary values
$arr1 = array(b"1", b"hello" => "hello", b"world", "str1" => b"hello", "str2" => "world");

// initialize the second argument
$arr2 = array(b"str1" => b"binary", b"hello" => "binary", b"str2" => b"binary");
 
echo "-- With default argument --\n";
var_dump( array_merge_recursive($arr1) );

echo "-- With more arguments --\n";
var_dump( array_merge_recursive($arr1, $arr2) );

echo "Done";
?>