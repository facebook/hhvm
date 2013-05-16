<?php
/* Prototype  : mixed current(array $array_arg)
 * Description: Return the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 * Alias to functions: pos
 */

/*
 * Test how the internal pointer is affected when two variables are referenced to each other
 */

echo "*** Testing current() : usage variations ***\n";

$array1 = array ('zero', 'one', 'two');

echo "\n-- Initial position of internal pointer --\n";
var_dump(current($array1));
next($array1);

// Test that when two variables are referenced to one another
// the internal pointer is the same for both
$array2 = &$array1;
echo "\n-- Position after calling next() --\n";
echo "\$array1: ";
var_dump(current($array1));
echo "\$array2: ";
var_dump(current($array2));
?>
===DONE===