<?php
/* Prototype  : bool sort ( array &$array [, int $sort_flags] )
 * Description: This function sorts an array. 
                Elements will be arranged from lowest to highest when this function has completed.
 * Source code: ext/standard/array.c
*/

/*
 * Testing sort() by providing different integer/float value arrays for $array argument
 * with following flag values
 * 1. flag  value as defualt
 * 2. SORT_REGULAR - compare items normally
 * 3. SORT_NUMERIC - compare items numerically
 * 4. SORT_STRING - compare items as strings
*/

echo "*** Testing sort() : usage variations ***\n";

// group of various arrays
$various_arrays = array (
  // negative/posative integers array
  array(11, -11, 21, -21, 31, -31, 0, 41, -41),

  // float value array
  array(10.5, -10.5, 10.5e2, 10.6E-2, .5, .01, -.1),

  // mixed value array
  array(.0001, .0021, -.01, -1, 0, .09, 2, -.9, 10.6E-2, -10.6E-2, 33),
 
  // array values contains minimum and maximum ranges
  array(2147483647, 2147483648, -2147483647, -2147483648, -0, 0, -2147483649)
);

// set of possible flag values
$flag_value = array("SORT_REGULAR" => SORT_REGULAR, "SORT_NUMERIC" => SORT_NUMERIC);

$count = 1;
echo "\n-- Testing sort() by supplying various integer/float arrays --\n";

// loop through to test sort() with different arrays
foreach ($various_arrays as $array) {
  echo "\n-- Iteration $count --\n";

  echo "- With Defualt sort flag -\n"; 
  $temp_array = $array; 
  var_dump(sort($temp_array) );
  var_dump($temp_array);

  // loop through $flag_value array and setting all possible flag values
  foreach($flag_value as $key => $flag){
    echo "- Sort flag = $key -\n";
    $temp_array = $array; 
    var_dump(sort($temp_array, $flag) );
    var_dump($temp_array);
  }  
  $count++;
} 

echo "Done\n";
?>