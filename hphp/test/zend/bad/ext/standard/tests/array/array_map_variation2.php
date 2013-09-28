<?php

/* Prototype  : array array_map(mixed callback, array input1 [, array input2 ,...])
 * Description: Applies the callback to the elements in given arrays.
 * Source code: ext/standard/array.c
*/

echo "*** Testing array_map() : references ***\n";
$arr = array("k1" => "v1","k2"=>"v2");
$arr[]=&$arr["k1"];
$arr[]=&$arr;
function cb1 ($a) {var_dump ($a);return array ($a);};
function cb2 (&$a) {var_dump ($a);return array (&$a);};
var_dump( array_map("cb1", $arr));
var_dump( array_map("cb2", $arr,$arr));
var_dump( array_map(null,  $arr));
var_dump( array_map(null, $arr, $arr));

// break cycles
$arr[0] = null;
$arr[1] = null;

echo "Done";
?>
