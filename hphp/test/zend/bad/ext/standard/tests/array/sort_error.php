<?php
/* Prototype  : bool sort(array &array_arg [, int sort_flags])
 * Description: Sort an array 
 * Source code: ext/standard/array.c
*/

/*
* Testing sort() function with all possible error conditions 
*/

echo "*** Testing sort() : error conditions ***\n";

// zero arguments
echo "\n-- Testing sort() function with Zero arguments --\n";
var_dump( sort() );

//Test sort with more than the expected number of arguments
echo "\n-- Testing sort() function with more than expected no. of arguments --\n";
$array_arg = array(1, 2);
$flag_value = array("SORT_REGULAR" => SORT_REGULAR, "SORT_STRING" => SORT_STRING, "SORT_NUMERIC" => SORT_NUMERIC);
$extra_arg = 10;

// loop through $flag_value array and setting all possible flag values
foreach($flag_value as $key => $flag){
  echo "\nSort flag = $key\n";
  var_dump( sort($array_arg,$flag, $extra_arg) );
   
  // dump the input array to ensure that it wasn't changed
  var_dump($array_arg);
}

echo "Done";
?>