<?php
/* Prototype  : bool ksort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key, maintaining key to data correlation 
 * Source code: ext/standard/array.c
*/

/*
 * testing ksort() by providing arrays containing sub arrays for $array argument 
 * with flowing flag values:
 *  1. flag value as defualt
 *  2. SORT_REGULAR - compare items normally
*/

echo "*** Testing ksort() : usage variations ***\n";

// array with diff sub arrays to be sorted 
$various_arrays = array (
  // null array
  1  => array(),

  // array contains null sub array
  2 => array( 1 => array() ),

  // array of arrays along with some values
  3 => array(4 => 44, 1 => 11, 3 => array(64,61) ),

  // array contains sub arrays
  4 => array ( 3 => array(33,-5,6), 1 => array(11), 
               2 => array(22,-55), 0  => array() )
);


$count = 1;
echo "\n-- Testing ksort() by supplying various arrays containing sub arrays --\n";

// loop through to test ksort() with different arrays
foreach ($various_arrays as $array) {
 
  echo "\n-- Iteration $count --\n"; 
  echo "- With defualt sort flag -\n";
  $temp_array = $array;
  var_dump( ksort($temp_array) );
  var_dump($temp_array);

  echo "- Sort flag = SORT_REGULAR -\n";
  $temp_array = $array;
  var_dump( ksort($temp_array, SORT_REGULAR) );
  var_dump($temp_array);
  $count++;
}

echo "Done\n";
?>