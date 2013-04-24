<?php
/* Prototype  : proto bool array_walk_recursive(array $input, string $funcname [, mixed $userdata])
 * Description: Apply a user function to every member of an array 
 * Source code: ext/standard/array.c
*/

/*
* Passing anonymous(run-time) callback function with following variations:
*   with one parameter
*   two parameters
*   three parameters
*   extra parameters
*   without parameters
*/

echo "*** Testing array_walk_recursive() : anonymous function as callback ***\n";

$input = array( array(2, 5), array(10, 0));

echo "-- Anonymous function with one argument --\n";
var_dump( array_walk_recursive($input, create_function('$value', 'var_dump($value); echo "\n";')));

echo "-- Anonymous function with two arguments --\n";
var_dump( array_walk_recursive($input, create_function('$value, $key', 'var_dump($key); var_dump($value); echo "\n";')));

echo "-- Anonymous function with three arguments --\n";
var_dump( array_walk_recursive($input, create_function('$value, $key, $user_data', 'var_dump($key); var_dump($value); var_dump($user_data); echo "\n";'), 10));

echo "-- Anonymous function with one more argument --\n";
var_dump( array_walk_recursive($input, create_function('$value, $key, $user_data', 'var_dump($key); var_dump($value); var_dump($user_data); echo "\n";'), 20, 30)); 

echo "-- Anonymous function with null argument --\n";
var_dump( array_walk_recursive( $input, create_function(null, 'echo "1\n";')));
echo "Done"
?>