<?php
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback. 
 * Source code: ext/standard/array.c
*/

/*
* Passing different anonymous callback functions with passed by value and reference arguments
*/

echo "*** Testing array_filter() : usage variations - Anonymous callback functions ***\n";

$input = array(0, 1, -1, 10, 100, 1000, 'Hello', null);

// anonymous callback function
echo "Anonymous callback function with regular parameter and statement\n";
var_dump( array_filter($input, create_function('$input', 'return ($input > 1);') ) );

// anonymous callback function with reference
echo "Anonymous callback function with reference parameter\n";
var_dump( array_filter($input, create_function('&$input', 'return ($input < 1);') ) );

// anonymous callback function with null argument
echo "Anonymous callback funciton with null argument\n";
var_dump( array_filter($input, create_function(null, 'return true;') ) );

// anonymous callback function with argument and null statement
echo "Anonymous callback function with regular argument and null statement\n";
var_dump( array_filter($input, create_function('$input', null) ) );

echo "Done"
?>