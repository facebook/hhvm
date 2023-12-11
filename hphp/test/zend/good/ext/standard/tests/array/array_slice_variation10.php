<?hh
/* Prototype  : array array_slice(array $input, int $offset [, int $length [, bool $preserve_keys]])
 * Description: Returns elements specified by offset and length
 * Source code: ext/standard/array.c
 */

/*
 * Check position of internal array pointer after calling array_slice()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_slice() : usage variations ***\n";

$input = dict['one' => 'un', 'two' => 'deux', 23 => 'twenty-three', 24 => 'zero'];

echo "\n-- Call array_slice() --\n";
var_dump($result = array_slice($input, 2));

echo "Done";
}
