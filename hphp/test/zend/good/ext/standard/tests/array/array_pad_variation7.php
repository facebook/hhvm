<?hh
/* Prototype  : array array_pad(array $input, int $pad_size, mixed $pad_value)
 * Description: Returns a copy of input array padded with pad_value to size pad_size
 * Source code: ext/standard/array.c
*/

/*
* Passing two dimensional array to $input argument and testing whether
* array_pad() behaves in an expected way with the other arguments passed to the function.
* The $pad_size and $pad_value arguments passed are fixed values.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_pad() : Passing 2-D array to \$input argument ***\n";

// initialize the 2-d array
$input = vec[
  vec[1, 2, 3],
  vec["hello", 'world'],
  dict["one" => 1, "two" => 2]
];

// initialize the $pad_size and $pad_value arguments
$pad_size = 5;
$pad_value = "HELLO";

// entire 2-d array
echo "-- Entire 2-d array for \$input argument --\n";
var_dump( array_pad($input, $pad_size, $pad_value) );  // positive 'pad_size'
var_dump( array_pad($input, -$pad_size, $pad_value) );  // negative 'pad_size'

// sub array
echo "-- Sub array for \$input argument --\n";
var_dump( array_pad($input[1], $pad_size, $pad_value) );  // positive 'pad_size'
var_dump( array_pad($input[1], -$pad_size, $pad_value) );  // negative 'pad_size'

echo "Done";
}
