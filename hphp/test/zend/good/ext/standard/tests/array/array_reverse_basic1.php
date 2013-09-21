<?php
/* Prototype  : array array_reverse(array $array [, bool $preserve_keys])
 * Description: Return input as a new array with the order of the entries reversed 
 * Source code: ext/standard/array.c
*/

/*
 * Testing array_reverse() by giving a simple array for $array argument 
*/

echo "*** Testing array_reverse() : basic functionality ***\n";

// Initialise the array
$array = array("a", "green", "red", 'blue', 10, 13.33);

// Calling array_reverse() with default arguments
var_dump( array_reverse($array) );

// Calling array_reverse() with all possible arguments
var_dump( array_reverse($array, true) );  // expects the keys to be preserved
var_dump( array_reverse($array, false) );  // expects the keys not to be preserved

echo "Done";
?>