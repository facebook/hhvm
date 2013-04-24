<?php
/* Prototype  : int count(mixed $var [, int $mode])
 * Description: Count the number of elements in a variable (usually an array) 
 * Source code: ext/standard/array.c
 */

/*
 * Pass count() an infinitely recursive array as $var argument
 * This will stop the script before it reaches the end.
 */

echo "*** Testing count() : usage variations ***\n";

$array1 = array (1, 2, 'three');
// get an infinitely recursive array
$array1[] = &$array1;

echo "\n-- \$mode not set: --\n";
var_dump(count ($array1));

echo "\n-- \$mode = 1: --\n";
var_dump(count ($array1, 1));

echo "Done";
?>