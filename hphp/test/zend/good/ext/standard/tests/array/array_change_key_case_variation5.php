<?hh
/* Prototype  : array array_change_key_case(array $input [, int $case])
 * Description: Retuns an array with all string keys lowercased [or uppercased]
 * Source code: ext/standard/array.c
 */

/*
 * Check the position of the internal array pointer after calling the function
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_change_key_case() : usage variations ***\n";

$input = dict['one' => 'un', 'two' => 'deux', 'three' => 'trois'];

echo "\n-- Call array_change_key_case() --\n";
var_dump($result = array_change_key_case($input, CASE_UPPER));

echo "Done";
}
