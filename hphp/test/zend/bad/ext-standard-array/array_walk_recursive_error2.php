<?php
/* Prototype  : bool array_walk_recursive(array $input, string $funcname [, mixed $userdata])
 * Description: Apply a user function to every member of an array 
 * Source code: ext/standard/array.c
*/

/*
 * Testing array_walk_recursive() by passing more number of parameters to callback function
 */
$input = array(1);

function callback1($value, $key, $user_data ) {
  echo "\ncallback1() invoked \n";
}

function callback2($value, $key, $user_data1, $user_data2) {
  echo "\ncallback2() invoked \n";
}
echo "*** Testing array_walk_recursive() : error conditions - callback parameters ***\n";

// expected: Missing argument Warning
var_dump( array_walk_recursive($input, "callback1") );
var_dump( array_walk_recursive($input, "callback2", 4) );

// expected: Warning is suppressed
var_dump( @array_walk_recursive($input, "callback1") );  
var_dump( @array_walk_recursive($input, "callback2", 4) );  

echo "-- Testing array_walk_recursive() function with too many callback parameters --\n";
var_dump( array_walk_recursive($input, "callback1", 20, 10) );

echo "Done";
?>