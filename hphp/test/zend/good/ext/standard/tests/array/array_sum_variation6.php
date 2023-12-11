<?hh
/* Prototype  : mixed array_sum(array $input)
 * Description: Returns the sum of the array entries
 * Source code: ext/standard/array.c
*/

/*
* Testing array_sum() with associative array as 'input' argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_sum() : with associative array ***\n";

// array with numeric keys
$input = dict[0 => 1, 1 => 10, 2 => 0, 3 => -2, 4 => 23.56];
echo "-- with numeric keys --\n";
var_dump( array_sum($input) );

// array with string keys
$input = dict['a' => 20, "b" => 50, 'c' => 0, 'd' => -30, "e" => 100];
echo "-- with string keys --\n";
var_dump( array_sum($input) );
echo "Done";
}
