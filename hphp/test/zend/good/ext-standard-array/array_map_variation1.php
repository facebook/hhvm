<?php

/* Prototype  : array array_map(mixed callback, array input1 [, array input2 ,...])
 * Description: Applies the callback to the elements in given arrays.
 * Source code: ext/standard/array.c
*/



echo "*** Testing array_map() : string keys ***\n";

$arr = array("stringkey" => "value");
function cb1 ($a) {return array ($a);};
function cb2 ($a,$b) {return array ($a,$b);};
var_dump( array_map("cb1", $arr));
var_dump( array_map("cb2", $arr,$arr));
var_dump( array_map(null,  $arr));
var_dump( array_map(null, $arr, $arr));
echo "Done";
?>
