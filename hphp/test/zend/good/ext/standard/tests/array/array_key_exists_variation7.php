<?hh
/* Prototype  : bool array_key_exists(mixed $key, array $search)
 * Description: Checks if the given key or index exists in the array
 * Source code: ext/standard/array.c
 * Alias to functions: key_exists
 */

/*
 * Check the position of the internal array pointer after calling array_key_exists()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_key_exists() : usage variations ***\n";

$input = dict['one' => 'un', 'two' => 'deux', 'three' => 'trois'];

echo "\n-- Call array_key_exists() --\n";
var_dump($result = array_key_exists('one', $input));

echo "Done";
}
