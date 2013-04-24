<?php
/* Prototype  : int array_push(array $stack, mixed $var [, mixed $...])
 * Description: Pushes elements onto the end of the array 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_push when passed:
 * 1. an array as $var arg
 * 2. as sub-array as $stack arg
 */

echo "*** Testing array_push() : usage variations ***\n";

echo "\n-- Pass array as \$var argument --\n";
$array = array(1, 2, 3);
$sub_array = array('one', 'two');
var_dump(array_push($array, $sub_array));
var_dump($array);

echo "\n-- Pass sub-array as \$stack argument --\n";
var_dump(array_push($array[3], 'a'));
var_dump($array);

echo "Done";
?>