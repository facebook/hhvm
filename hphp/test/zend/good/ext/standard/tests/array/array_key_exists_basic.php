<?hh
/* Prototype  : bool array_key_exists(mixed $key, array $search)
 * Description: Checks if the given key or index exists in the array
 * Source code: ext/standard/array.c
 * Alias to functions: key_exists
 */

/*
 * Test basic functionality of array_key_exists()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_key_exists() : basic functionality ***\n";

$key1 = 'key';
$key2 = 'val';
$search = dict[0 => 'one', 'key' => 'value', 1 => 'val'];
var_dump(array_key_exists($key1, $search));
var_dump(array_key_exists($key2, $search));

echo "Done";
}
