<?hh
/* Prototype  : array array_values(array $input)
 * Description: Return just the values from the input array 
 * Source code: ext/standard/array.c
 */

/*
 * Check that array_values is re-assigning keys according to the internal order of the array,
 * and is not dependent on the \$input argument's keys
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_values() : usage variations ***\n";

// populate array with 'default' keys in reverse order
$input = dict[3 => 'three', 2 => 'two', 1 => 'one', 0 => 'zero'];

echo "\n-- \$input argument: --\n";
var_dump($input);

echo "\n-- Result of array_values() --\n";
var_dump(array_values($input));

echo "Done";
}
