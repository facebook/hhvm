<?php
/* Prototype  : mixed array_shift(array &$stack)
 * Description: Pops an element off the beginning of the array 
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of array_shift()
 */

echo "*** Testing array_shift() : basic functionality ***\n";

$array = array('zero', 'one', '3' => 'three', 'four' => 4);
echo "\n-- Before shift: --\n";
var_dump($array);

echo "\n-- After shift: --\n";
echo "Returned value:\t";
var_dump(array_shift($array));
echo "New array:\n";
var_dump($array);

echo "Done";
?>