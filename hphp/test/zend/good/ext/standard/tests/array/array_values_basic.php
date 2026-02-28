<?hh
/* Prototype  : array array_values(array $input)
 * Description: Return just the values from the input array
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of array_values()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_values() : basic functionality ***\n";


// Initialise all required variables
$input = dict[0 => 'zero', 1 => 'one', 2 => 'two', 'three' => 3, 10 => 'ten'];

// Calling array_values() with all possible arguments
var_dump( array_values($input) );

echo "Done";
}
