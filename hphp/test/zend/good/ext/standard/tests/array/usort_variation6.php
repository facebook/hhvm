<?php
/* Prototype  : bool usort(array $array_arg, string $cmp_function)
 * Description: Sort an array by values using a user-defined comparison function 
 * Source code: ext/standard/array.c
 */

/*
 * Pass a multi-dimensional array as $array_arg argument to usort()
 * to test how array is re-ordered
 */

echo "*** Testing usort() : usage variation ***\n";

function cmp_function($value1, $value2)
{
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 > $value2) {
    return 1;
  }
  else {
    return -1;
  }
}

$array_args = array(
  0 => array(2, 10, -1),
  1 => array(100),
  2 => array(),
  3 => array(0),
  4 => array(-1),
  5 => array(-9, 34, 54, 0, 20),
  6 => array(''),
  7 => array("apple", "Apple", "APPLE", "aPPle", "aPpLe")
);

$temp_array = $array_args;

echo "\n-- Pass usort() a two-dimensional array --\n";
// sorting array_arg as whole array
var_dump( usort($temp_array, 'cmp_function') );

echo "-- Array after call to usort() --\n";
var_dump($temp_array);

echo "\n-- Pass usort() a sub-array --\n";
var_dump( usort($array_args[5], 'cmp_function') );

echo "-- Array after call to usort() --\n";
var_dump($array_args[5]);
?>
===DONE===