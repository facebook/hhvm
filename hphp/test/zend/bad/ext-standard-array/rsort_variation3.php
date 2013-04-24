<?php
/* Prototype  : bool rsort(array &$array_arg [, int $sort_flags])
 * Description: Sort an array in reverse order 
 * Source code: ext/standard/array.c
 */

/*
 * Pass arrays containing different numeric data to rsort() to test behaviour
 */

echo "*** Testing rsort() : variation ***\n";

// group of various arrays

$various_arrays = array (
// negative/positive integers array
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

// loop through to test rsort() with different arrays
foreach ($various_arrays as $array) {
  echo "\n-- Iteration $count --\n";

  echo "- With Defualt sort flag -\n"; 
  $temp_array = $array; 
  var_dump(rsort($temp_array) );
  var_dump($temp_array);

  // loop through $flag_value array and setting all possible flag values
  foreach($flag_value as $key => $flag){
    echo "- Sort flag = $key -\n";
    $temp_array = $array; 
    var_dump(rsort($temp_array, $flag) );
    var_dump($temp_array);
  }  
  $count++;
} 

echo "Done";
?>
