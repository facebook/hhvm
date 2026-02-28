<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing different callback function returning:
 *   int, string, bool, null values
 */
function callback_int($a, $b)
:mixed{
  return $a + $b;
}
function callback_string($a, $b)
:mixed{
  return "$a"."$b";
}
function callback_bool($a, $b)
:mixed{
  return TRUE;
}
function callback_null($array1)
:mixed{
  return NULL;
}
function callback_without_ret($arr1)
:mixed{
  echo "callback_without_ret called\n";
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : callback with diff return value ***\n";

$array1 = vec[1, 2, 3];
$array2 = vec[3, 4, 5];

echo "-- with integer return value --\n";
var_dump( array_map(callback_int<>, $array1, $array2));

echo "-- with string return value --\n";
var_dump( array_map(callback_string<>, $array1, $array2));

echo "-- with bool return value --\n";
var_dump( array_map(callback_bool<>, $array1, $array2));

echo "-- with null return value --\n";
var_dump( array_map(callback_null<>, $array1));

echo "-- with no return value --\n";
var_dump( array_map(callback_without_ret<>, $array1));

echo "Done";
}
