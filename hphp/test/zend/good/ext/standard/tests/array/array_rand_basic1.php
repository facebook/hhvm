<?hh
/* Prototype  : mixed array_rand(array $input [, int $num_req])
 * Description: Return key/keys for random entry/entries in the array 
 * Source code: ext/standard/array.c
*/

/*
 * Test array_rand() when array with default keys is passed to 'input' argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_rand() : array with default keys ***\n";


// Initialise the 'input' and 'num_req' variables
$input = vec[10, 20, 30, 40, 50, 60, 70];
$num_req = 6;

// Calling array_rand() with optional argument
echo"\n-- with all default and optional arguments --\n";
var_dump( array_rand($input,$num_req) );

// Calling array_rand() with default arguments
echo"\n-- with default argument --\n";
var_dump( array_rand($input) );

echo "Done";
}
