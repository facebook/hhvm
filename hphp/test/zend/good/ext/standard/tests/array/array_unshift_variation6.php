<?hh
/* Prototype  : int array_unshift(array $array, mixed $var [, mixed ...])
 * Description: Pushes elements onto the beginning of the array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_unshift() by giving two-dimensional
 * arrays and also sub-arrays within the two-dimensional array for $array argument.
 * The $var argument passed is a fixed value
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_unshift() : two dimensional arrays for \$array argument ***\n";

// initializing $var argument
$var = 10;

// two-dimensional array to be passed to $array argument
$two_dimensional_array = vec[

   // associative array
   dict['color' => 'red', 'item' => 'pen', 'place' => 'LA'],

   // numeric array
   vec[1, 2, 3, 4, 5],

   // combination of numeric and associative arrays
   dict['a' => 'green', 0 => 'red', 1 => 'brown', 2 => 33, 3 => 88, 4 => 'orange', 'item' => 'ball']
];

/* Passing the entire $two_dimensional_array to $array */

/* With default argument */
// returns element count in the resulting array after arguments are pushed to
// beginning of the given array
$temp_array = $two_dimensional_array;
var_dump( array_unshift(inout $temp_array, $var) );  // whole 2-d array

// dumps the resulting array
var_dump($temp_array);

/* With optional arguments */
// returns element count in the resulting array after arguments are pushed to
// beginning of the given array
$temp_array = $two_dimensional_array;
var_dump( array_unshift(inout $temp_array, $var, "hello", 'world') );  // whole 2-d array

// dumps the resulting array
var_dump($temp_array);

/* Passing the sub-array within the $two_dimensional_array to $array argument */

/* With default argument */
// returns element count in the resulting array after arguments are pushed to
// beginning of the given array
$temp_array = $two_dimensional_array[0];
var_dump( array_unshift(inout $temp_array, $var) );  // sub array

// dumps the resulting array
var_dump($temp_array);

/* With optional arguments */
// returns element count in the resulting array after arguments are pushed to
// beginning of the given array
$temp_array = $two_dimensional_array[0];
var_dump( array_unshift(inout $temp_array, $var, "hello", 'world') );  // sub array

// dumps the resulting array
var_dump($temp_array);

echo "Done";
}
