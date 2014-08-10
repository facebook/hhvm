<?php
/* Prototype  : array array_intersect_uassoc(array arr1, array arr2 [, array ...], callback key_compare_func)
 * Description: Computes the intersection of arrays with additional index check, compares indexes by a callback function
 * Source code: ext/standard/array.c
 */

echo "*** Testing array_intersect_uassoc() : usage variation ***\n";

//Initialize variables
$array1 = array("a" => "green", "b" => "brown", "c" => "blue", "red");
$array2 = array("a" => "green", "yellow", "red");

//function name within double quotes
var_dump( array_intersect_uassoc($array1, $array2, "unknown_function") );

//function name within single quotes
var_dump( array_intersect_uassoc($array1, $array2, 'unknown_function') );

//function name without quotes
var_dump( array_intersect_uassoc($array1, $array2, unknown_function) );
?>
===DONE===
