<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing array having different subarrays
 */

function callback($a)
{
  return $a;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : array having subarrays ***\n";

// different subarrays
$arr1 = array(
  array(),
  array(1, 2),
  array('a', 'b'),
  array(1, 2, 'a', 'b'),
  array(1 => 'a', 'b' => 2)
);

var_dump( array_map(fun('callback'), $arr1));
echo "Done";
}
