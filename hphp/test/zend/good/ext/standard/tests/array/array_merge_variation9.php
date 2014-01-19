<?php
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array 
 * Source code: ext/standard/array.c
 */

/* Test array_merge() when:
 * 1. Passed an array made up of referenced variables
 * 2. Passed an array as the first argument and a reference to that array as the second.
 */

echo "*** Testing array_merge() : usage variations ***\n";

$val1 = 'foo';
$val2 = 'bar';
$val3 = 'baz';

$arr1 = array(&$val1, &$val2, &$val3);
$arr2 = array('key1' => 'val1', 'key2' => 'val2', 'key3' => 'val3');

echo "\n-- Merge an array made up of referenced variables to an assoc. array --\n";
var_dump(array_merge($arr1, $arr2));
var_dump(array_merge($arr2, $arr1));

$val2 = 'hello world';

echo "\n-- Change \$val2 --\n";
var_dump(array_merge($arr1, $arr2));
var_dump(array_merge($arr2, $arr1));

echo "Done";
?>
