<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing different callback function returning:
 *   int, string, bool, null values
 */

echo "*** Testing array_map() : callback with diff return value ***\n";

$array1 = array(1, 2, 3);
$array2 = array(3, 4, 5);

echo "-- with integer return value --\n";
function callback_int($a, $b)
{
  return $a + $b;
}
var_dump( array_map('callback_int', $array1, $array2));

echo "-- with string return value --\n";
function callback_string($a, $b)
{
  return "$a"."$b";
}
var_dump( array_map('callback_string', $array1, $array2));

echo "-- with bool return value --\n";
function callback_bool($a, $b)
{
  return TRUE;
}
var_dump( array_map('callback_bool', $array1, $array2));

echo "-- with null return value --\n";
function callback_null($array1)
{
  return NULL;
}
var_dump( array_map('callback_null', $array1));

echo "-- with no return value --\n";
function callback_without_ret($arr1)
{
  echo "callback_without_ret called\n";
}
var_dump( array_map('callback_without_ret', $array1));

echo "Done";
?>