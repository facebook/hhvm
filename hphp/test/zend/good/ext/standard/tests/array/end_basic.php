<?php
/* Prototype  : mixed end(array $array_arg)
 * Description: Advances array argument's internal pointer to the last element and return it 
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of end()
 */

echo "*** Testing end() : basic functionality ***\n";

$array = array('zero', 'one', 200 => 'two');

echo "\n-- Initial Position: --\n";
echo key($array) . " => " . current($array) . "\n";

echo "\n-- Call to end() --\n";
var_dump(end($array));

echo "\n-- Current Position: --\n";
echo key($array) . " => " . current($array) . "\n";

echo "\n-- Add a new element to array --\n";
$array[2] = 'foo';
var_dump(end($array));
?>
===DONE===