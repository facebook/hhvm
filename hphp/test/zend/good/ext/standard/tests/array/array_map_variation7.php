<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing array having different size
 *   1) first array as empty array
 *   2) second array as empty array
 *   3) second array shorter than first array
 *   4) first array shorter than second array
 *   5) one more array than callback function arguments
 */

function callback($a, $b)
:mixed{
  return vec[$a, $b];
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : arrays with diff. size ***\n";

// calling array_map with different arrays
var_dump( array_map(callback<>, vec[1, 2, 3], vec[]) );
var_dump( array_map(callback<>, vec[], vec['a', 'b', 'c']) );
var_dump( array_map(callback<>, vec[1, 2, 3], vec['a', 'b']) );
var_dump( array_map(callback<>, vec[012, 0x2F, 0X1A], vec[2.3, 12.4e2]) );
var_dump( array_map(callback<>, vec[], vec[1, 2, 3], vec['a', 'b']) );  // passing more no. of arrays than callback function argument

echo "Done";
}
