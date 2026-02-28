<?hh
/* Prototype  : array array_reverse(array $array [, bool $preserve_keys])
 * Description: Return input as a new array with the order of the entries reversed
 * Source code: ext/standard/array.c
*/

/*
 * testing the functionality of array_reverse() by giving 2-D arrays for $array argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_reverse() : usage variations ***\n";

// Initializing the 2-d arrays
$two_dimensional_array = vec[

  // associative array
  dict['color' => 'red', 'item' => 'pen', 'place' => 'LA'],

   // numeric array
   vec[1, 2, 3, 4, 5],

   // combination of numeric and associative arrays
   dict['a' => 'green', 0 => 'red', 1 => 'brown', 2 => 33, 3 => 88, 4 => 'orange', 'item' => 'ball']
];

// calling array_reverse() with various types of 2-d arrays
// with default arguments
echo "-- with default argument --\n";
var_dump( array_reverse($two_dimensional_array) );  // whole array
var_dump( array_reverse($two_dimensional_array[1]) );  // sub array

// with $preserve_keys argument
echo "-- with all possible arguments --\n";
// whole array
var_dump( array_reverse($two_dimensional_array, true) );
var_dump( array_reverse($two_dimensional_array, false) );
// sub array
var_dump( array_reverse($two_dimensional_array[1], true) );
var_dump( array_reverse($two_dimensional_array[1], false) );

echo "Done";
}
