<?php
/* Prototype  : bool array_walk(array $input, string $funcname [, mixed $userdata])
 * Description: Apply a user function to every member of an array 
 * Source code: ext/standard/array.c
*/

/*
 * Passing different buit-in functionns as callback function
 *    pow function
 *    min function
 *    echo language construct
*/

echo "*** Testing array_walk() : built-in function as callback ***\n";

$input = array(2 => 1, 65, 98, 100, 6 => -4);

echo "-- With 'pow' built-in function --\n";
var_dump( array_walk($input, 'pow'));

echo "-- With 'min' built-in function --\n";
var_dump( array_walk($input, "min"));

echo "-- With 'echo' language construct --\n";
var_dump( array_walk($input, "echo"));

echo "Done"
?>