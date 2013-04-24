<?php
/* Prototype  : bool array_walk_recursive(array $input, string $funcname [, mixed $userdata])
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

echo "*** Testing array_walk_recursive() : error conditions ***\n";

echo "-- Testing array_walk_recursive() function with zero arguments --\n";
var_dump( array_walk_recursive() );

echo "-- Testing array_walk_recursive() function with one argument --\n";
var_dump( array_walk_recursive($input) );

$input = array( array(1, 2), array(3), array(4, 5));
echo "-- Testing array_walk_recursive() function with non existent callback function  --\n";
var_dump( array_walk_recursive($input, "non_existent") );

echo "Done";
?>