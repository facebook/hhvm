<?php
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length 
 * Source code: ext/standard/array.c
 */

/*
 * Check position of internal array pointer after calling array_slice()
 */

echo "*** Testing array_slice() : usage variations ***\n";

$input = array ('one' => 'un', 'two' => 'deux', 23 => 'twenty-three', 'zero');

echo "\n-- Call array_slice() --\n";
var_dump($result = array_slice($input, 2));

echo "-- Position of Internal Pointer in Result: --\n";
echo key($result) . " => " . current($result) . "\n";
echo "\n-- Position of Internal Pointer in Original Array: --\n";
echo key($input) . " => " . current ($input) . "\n";

echo "Done";
?>
