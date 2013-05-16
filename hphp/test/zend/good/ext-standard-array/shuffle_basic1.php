<?php
/* Prototype  : bool shuffle(array $array_arg)
 * Description: Randomly shuffle the contents of an array 
 * Source code: ext/standard/array.c
*/

/*
* Test behaviour of shuffle when an array with default keys
* is passed to the 'array_arg' argument and check for the
* changes in the input array by printing the input array
* before and after shuffle() function is applied on it
*/

echo "*** Testing shuffle() : with arrays having default keys ***\n";

// Initialise the array with integers
$array_arg_int = array(0, 10, 20, 30, 40, 50, 60, 70, 80);

// Initialise the array with strings
$array_arg_strings = array("one", 'two', 'three', "four", "five", " ", 'six', ' ', "seven");

/* Testing shuffle() function with array of integers */

// printing the input array with integers before the shuffle operation
echo "\n-- input array of integers before shuffle() function is applied --\n";
var_dump( $array_arg_int );

// applying shuffle() function on the input array of integers
echo "\n-- return value from shuffle() function --\n";
var_dump( shuffle($array_arg_int) );  // prints the return value from shuffle() function

echo "\n-- resultant array after shuffle() function is applied --\n";
var_dump( $array_arg_int );

/* Testing shuffle() function with array of strings */

// printing the input array with strings before the shuffle operation
echo "\n-- input array of strings before shuffle() function is applied --\n";
var_dump( $array_arg_strings );

// applying shuffle() function on the input array of strings
echo "\n-- return value from shuffle() function --\n";
var_dump( shuffle($array_arg_strings) );  // prints the return value from shuffle() function

echo "\n-- resultant array after shuffle() function is applied --\n";
var_dump( $array_arg_strings );

echo "Done";
?>