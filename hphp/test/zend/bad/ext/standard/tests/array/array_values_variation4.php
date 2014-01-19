<?php
/* Prototype  : array array_values(array $input)
 * Description: Return just the values from the input array 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_values when:
 * 1. Passed a two-dimensional array as $input argument
 * 2. Passed a sub-array as $input argument
 * 3. Passed an infinitely recursive multi-dimensional array
 */

echo "*** Testing array_values() : usage variations ***\n";

$input = array ('zero' => 'zero', 'un' => 'one', 'sub' => array (1, 2, 3));

echo "\n-- Array values of a two-dimensional array --\n";
var_dump(array_values($input));

echo "\n-- Array values of a sub-array --\n";
var_dump(array_values($input['sub']));

// get an infinitely recursive array
$input[] = &$input;
echo "\n-- Array values of an infinitely recursive array --\n";
var_dump(array_values($input));

echo "Done";
?>
