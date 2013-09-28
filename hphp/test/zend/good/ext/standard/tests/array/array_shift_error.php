<?php
/* Prototype  : mixed array_shift(array &$stack)
 * Description: Pops an element off the beginning of the array 
 * Source code: ext/standard/array.c 
 */

/*
 * Pass incorrect number of arguments to array_shift() to test behaviour
 */

echo "*** Testing array_shift() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_shift() function with Zero arguments --\n";
var_dump( array_shift() );

//Test array_shift with one more than the expected number of arguments
echo "\n-- Testing array_shift() function with more than expected no. of arguments --\n";
$stack = array(1, 2);
$extra_arg = 10;
var_dump( array_shift($stack, $extra_arg) );

echo "Done";
?>