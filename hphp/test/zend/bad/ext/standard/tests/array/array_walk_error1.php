<?php
/* Prototype  : bool array_walk(array $input, string $funcname [, mixed $userdata])
 * Description: Apply a user function to every member of an array 
 * Source code: ext/standard/array.c
*/

$input = array(1, 2);

/* Prototype : callback(mixed value, mixed key, mixed user_data)
 * Parameters : value - value in key/value pair
 *              key - key in key/value pair
 *              user_data - extra parameter
 */
function callback ($value, $key, $user_data) {
  echo "\ncallback() invoked \n";
}

echo "*** Testing array_walk() : error conditions ***\n";

echo "-- Testing array_walk() function with zero arguments --\n";
var_dump( array_walk() );

echo "-- Testing array_walk() function with one argument --\n";
var_dump( array_walk($input) );

echo "-- Testing array_walk() function with non existent callback function  --\n";
var_dump( array_walk($input, "non_existent") );

echo "Done";
?>