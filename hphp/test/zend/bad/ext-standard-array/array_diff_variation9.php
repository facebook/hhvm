<?php
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of arr1 that have values which are 
 * not present in any of the others arguments. 
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff() compares multidimensional arrays
 */

echo "*** Testing array_diff() : usage variations ***\n";

$array1 = array('sub_array1' => array (1, 2, 3),
                'sub_array2' => array ('a', 'b', 'c'));
$array2 = array('sub_arraya' => array (1, 3, 5),
                'sub_arrayb' => array ('a', 'z', 'y'));

echo "-- Compare two 2-D arrays --\n";
var_dump(array_diff($array1, $array2));
var_dump(array_diff($array2, $array1));

echo "\n-- Compare subarrays from two 2-D arrays --\n";
var_dump(array_diff($array1['sub_array1'], $array2['sub_arraya']));
var_dump(array_diff($array2['sub_arraya'], $array1['sub_array1']));

var_dump(array_diff($array1['sub_array2'], $array2['sub_arrayb']));
var_dump(array_diff($array2['sub_arrayb'], $array1['sub_array1']));

echo "\n-- Compare a subarray from one 2-D array and one 2-D array --\n";
var_dump(array_diff($array1['sub_array1'], $array2));
var_dump(array_diff($array1, $array2['sub_arraya']));

echo "Done";
?>