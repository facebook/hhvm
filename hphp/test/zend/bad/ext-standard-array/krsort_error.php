<?php
/* Prototype  : bool krsort(array &array_arg [, int asort_flags])
 * Description: Sort an array 
 * Source code: ext/standard/array.c
*/

/*
* Testing krsort() function with all possible error conditions 
*/

echo "*** Testing krsort() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing krsort() function with zero arguments --\n";
var_dump( krsort() );

//Test krsort with more than the expected number of arguments
echo "\n-- Testing krsort() function with more than expected no. of arguments --\n";
$array_arg = array(1 => 1, 2 => 2);
$flags = array("SORT_REGULAR" => SORT_REGULAR, "SORT_STRING" => SORT_STRING, "SORT_NUMERIC" => SORT_NUMERIC);
$extra_arg = 10;

// loop through $flag_value array and call krsort with all possible sort flag values
foreach($flags as $key => $flag){
  echo "\n- Sort flag = $key -\n";
  $temp_array = $array_arg;
  var_dump( krsort($temp_array,$flag, $extra_arg) );
  var_dump($temp_array); 
}

echo "Done";
?>