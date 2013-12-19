<?php
/* Prototype: array array_reduce(array $array, mixed $callback, mixed $initial);
   Description: Iteratively reduce the array to a single value via the callback
*/

$array = array('foo', 'foo', 'bar', 'qux', 'qux', 'quux');

echo "\n*** Testing array_reduce() to integer ***\n";
function reduce_int($w, $v) { return $w + strlen($v); }
$initial = 42;
var_dump(array_reduce($array, 'reduce_int', $initial), $initial);

echo "\n*** Testing array_reduce() to float ***\n";
function reduce_float($w, $v) { return $w + strlen($v) / 10; }
$initial = 4.2;
var_dump(array_reduce($array, 'reduce_float', $initial), $initial);

echo "\n*** Testing array_reduce() to string ***\n";
function reduce_string($w, $v) { return $w . $v; }
$initial = 'quux';
var_dump(array_reduce($array, 'reduce_string', $initial), $initial);

echo "\n*** Testing array_reduce() to array ***\n";
function reduce_array($w, $v) { $w[$v]++; return $w; }
$initial = array('foo' => 42, 'bar' => 17, 'qux' => -2, 'quux' => 0);
var_dump(array_reduce($array, 'reduce_array', $initial), $initial);

echo "\n*** Testing array_reduce() to null ***\n";
function reduce_null($w, $v) { return $w . $v; }
$initial = null;
var_dump(array_reduce($array, 'reduce_null', $initial), $initial);

echo "\nDone";
?> 