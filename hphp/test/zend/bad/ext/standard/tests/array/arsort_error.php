<?php
/* Prototype  : bool arsort(array &array_arg [, int sort_flags])
 * Description: Sort an array 
 * Source code: ext/standard/array.c
*/

/*
* Testing arsort() function with all possible error conditions 
*/

echo "*** Testing arsort() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing arsort() function with Zero arguments --\n";
var_dump( arsort() );

//Test arsort with more than the expected number of arguments
echo "\n-- Testing arsort() function with more than expected no. of arguments --\n";
$array_arg = array(1, 2);
$flags = array("SORT_REGULAR" => SORT_REGULAR, "SORT_STRING" => SORT_STRING, "SORT_NUMERIC" => SORT_NUMERIC);
$extra_arg = 10;

// loop through $flag_value array and setting all possible flag values
foreach($flags as $key => $flag){
  echo "\nSort flag = $key\n";
  var_dump( arsort($array_arg,$flag, $extra_arg) );

  // dump the input array to ensure that it wasn't changed
  var_dump($array_arg);
}

echo "Done";
?>
