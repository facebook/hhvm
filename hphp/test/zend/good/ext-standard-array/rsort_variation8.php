<?php
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order 
 * Source code: ext/standard/array.c
 */

/*
 * Pass rsort() multi-dimensional arrays to test behaviour
 */

echo "*** Testing rsort() : variation ***\n";

// array of arrays
$various_arrays = array (
  // null array
  array(),

  // array contains null sub array
  array( array() ),

  // array of arrays along with some values
  array(44, 11, array(64, 61) ),

  // array containing sub arrays
  array(array(33, -5, 6), array(11), array(22, -55), array() )
);


$count = 1;

// loop through to test rsort() with different arrays
foreach ($various_arrays as $array) {
 
  echo "\n-- Iteration $count --\n"; 
  
  echo "\n-- 'flag' value is default --\n";
  $temp_array = $array;
  var_dump(rsort($temp_array) );
  var_dump($temp_array);
  
  echo "\n-- 'flag' value is SORT_REGULAR --\n";
  $temp_array = $array;
  var_dump(rsort($temp_array, SORT_REGULAR) );
  var_dump($temp_array);
  $count++;
}

echo "Done";
?>
