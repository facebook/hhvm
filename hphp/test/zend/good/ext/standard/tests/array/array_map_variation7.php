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
  return varray[$a, $b];
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : arrays with diff. size ***\n";

// calling array_map with different arrays
var_dump( array_map(callback<>, varray[1, 2, 3], varray[]) );
var_dump( array_map(callback<>, varray[], varray['a', 'b', 'c']) );
var_dump( array_map(callback<>, varray[1, 2, 3], varray['a', 'b']) );
var_dump( array_map(callback<>, varray[012, 0x2F, 0X1A], varray[2.3, 12.4e2]) );
var_dump( array_map(callback<>, varray[], varray[1, 2, 3], varray['a', 'b']) );  // passing more no. of arrays than callback function argument

echo "Done";
}
