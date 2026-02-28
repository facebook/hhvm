<?hh
/* Prototype  : array array_values(array $input)
 * Description: Return just the values from the input array
 * Source code: ext/standard/array.c
 */

/*
 * Test the position of the internal array pointer after a call to array_values
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_values() : usage variations ***\n";

$input = dict['one' => 'un', 'two' => 'deux', 'three' => 'trois'];

echo "\n-- Call array_values() --\n";
var_dump($result = array_values($input));

echo "Done";
}
