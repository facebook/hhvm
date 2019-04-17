<?php
/* Prototype  : mixed array_shift(array &$stack)
 * Description: Pops an element off the beginning of the array
 * Source code: ext/standard/array.c
 */

/*
 * Test popping elements from a sub-array and popping an array from an array
 */

echo "*** Testing array_shift() : usage variations ***\n";

$stack_first = array(array(1, 2, 3), 'one', 'two');
$last = array (1, 2, 3);
$stack_last = array ('zero', 'one', $last );
echo "\n-- Before shift: --\n";
echo "---- \$stack_first:\n";
var_dump($stack_first);
echo "---- \$stack_last:\n";
var_dump($stack_last);

echo "\n-- After shift: --\n";
echo "---- Pop array from array:\n";
echo "Returned value:\t";
var_dump(array_shift(&$stack_first));
echo "New array:\n";
var_dump($stack_first);

echo "---- Pop element from array within array:\n";
echo "Returned value:\t";
var_dump(array_shift(&$last));
$stack_last[2] = $last;
echo "New array:\n";
var_dump($stack_last);

echo "Done";
