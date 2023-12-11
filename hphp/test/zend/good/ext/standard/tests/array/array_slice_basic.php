<?hh
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of array_slice()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_slice() : basic functionality ***\n";


$input = dict['one' => 1, 'two' => 2, 0 => 3, 23 => 4];
$offset = 2;
$length = 2;
$preserve_keys = true;

// Calling array_slice() with all possible arguments
echo "\n-- All arguments --\n";
var_dump( array_slice($input, $offset, $length, $preserve_keys) );

// Calling array_slice() with mandatory arguments
echo "\n-- Mandatory arguments --\n";
var_dump( array_slice($input, $offset) );

echo "Done";
}
