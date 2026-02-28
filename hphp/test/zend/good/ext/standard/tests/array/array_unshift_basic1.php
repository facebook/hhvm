<?hh
/* Prototype  : int array_unshift(&array $array, mixed $var [, mixed ...])
 * Description: Pushes elements onto the beginning of the array
 * Source code: ext/standard/array.c
*/

/*
 * Testing array_unshift() by giving array with default keys for $array argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_unshift() : basic functionality with default key array ***\n";

// Initialise the array
$array = vec[1, 2];

// Calling array_unshift() with default argument
$temp_array = $array;
// returns element count in the resulting array after arguments are pushed to
// beginning of the given array
var_dump( array_unshift(inout $temp_array, 10) );

// dump the resulting array
var_dump($temp_array);

// Calling array_unshift() with optional arguments
$temp_array = $array;
// returns element count in the resulting array after arguments are pushed to
// beginning of the given array
var_dump( array_unshift(inout $temp_array, 222, "hello", 12.33) );

// dump the resulting array
var_dump($temp_array);

echo "Done";
}
