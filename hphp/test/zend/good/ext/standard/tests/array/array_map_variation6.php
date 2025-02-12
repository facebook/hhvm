<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing array having different subarrays
 */

function callback($a)
:mixed{
  return $a;
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : array having subarrays ***\n";

// different subarrays
$arr1 = vec[
  vec[],
  vec[1, 2],
  vec['a', 'b'],
  vec[1, 2, 'a', 'b'],
  dict[1 => 'a', 'b' => 2]
];

var_dump( array_map(callback<>, $arr1));
echo "Done";
}
