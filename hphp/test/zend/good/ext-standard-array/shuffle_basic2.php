<?php
/* Prototype  : bool shuffle(array $array_arg)
 * Description: Randomly shuffle the contents of an array 
 * Source code: ext/standard/array.c
*/

/*
* Test behaviour of shuffle when an associative array is 
* passed to the 'array_arg' argument and check for the
* changes in the input array by printing the input array
* before and after shuffle() function is applied on it
*/

echo "*** Testing shuffle() : with associative array ***\n";

// Initialise the associative array
$array_arg = array(
  'one' => 1, 2 => 02, 'three' => 3,
   4 => 4, '#5' => 5, 'SIX' => 6,
  "seven" => 0x7, "#8" => 012, "nine" => 9
);

// printing the input array before the shuffle operation
echo "\n-- input array before shuffle() function is applied --\n";
var_dump( $array_arg );

// applying shuffle() function on the input array
echo "\n-- return value from shuffle() function --\n";
var_dump( shuffle($array_arg) );  // prints the return value from shuffle() function

echo "\n-- resultant array after shuffle() function is applied --\n";
var_dump( $array_arg );

echo "Done";
?>