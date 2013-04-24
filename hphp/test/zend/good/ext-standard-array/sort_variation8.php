<?php
/* Prototype  : bool sort ( array &$array [, int $sort_flags] )
 * Description: This function sorts an array. 
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * testing sort() by providing arrays contains sub arrays for $array argument with flowing flag values
 * flag value as defualt
 * SORT_REGULAR - compare items normally
*/

echo "*** Testing sort() : usage variations ***\n";

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
echo "\n-- Testing sort() by supplying various arrays containing sub arrays --\n";

// loop through to test sort() with different arrays
foreach ($various_arrays as $array) {
 
  echo "\n-- Iteration $count --\n"; 
  // testing sort() function by supplying different arrays, flag value is defualt
  echo "- With Defualt sort flag -\n";
  $temp_array = $array;
  var_dump(sort($temp_array) );
  var_dump($temp_array);

  // testing sort() function by supplying different arrays, flag value = SORT_REGULAR
  echo "- Sort flag = SORT_REGULAR -\n";
  $temp_array = $array;
  var_dump(sort($temp_array, SORT_REGULAR) );
  var_dump($temp_array);
  $count++;
}

echo "Done\n";
?>