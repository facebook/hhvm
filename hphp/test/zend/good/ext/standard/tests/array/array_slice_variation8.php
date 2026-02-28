<?hh
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length
 * Source code: ext/standard/array.c
 */

/*
 * Test array_slice when passed
 * 1. a two-dimensional array as $input argument
 * 2. a sub-array as $input argument
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_slice() : usage variations ***\n";

$input = dict[0 => 'zero', 1 => 'one', 2 => vec['zero', 'un', 'deux'], 9 => 'nine'];

echo "\n-- Slice a two-dimensional array --\n";
var_dump(array_slice($input, 1, 3));

echo "\n-- \$input is a sub-array --\n";
var_dump(array_slice($input[2], 1, 2));

echo "Done";
}
