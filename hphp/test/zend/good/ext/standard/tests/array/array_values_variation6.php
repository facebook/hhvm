<?php
/* Prototype  : array array_values(array $input)
 * Description: Return just the values from the input array 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_values() when:
 * 1. Passed an array made up of referenced variables
 * 2. Passed an array by reference
 */

echo "*** Testing array_values() : usage variations ***\n";

$val1 = 'one';
$val2 = 'two';
$val3 = 'three';

echo "\n-- \$input is an array made up of referenced variables: --\n";
$input = array(&$val1, &$val2, &$val3);
var_dump($result1 = array_values($input));

echo "Change \$val2 and check result of array_values():\n";
$val2 = 'deux';
var_dump($result1);

echo "Done";
?>
