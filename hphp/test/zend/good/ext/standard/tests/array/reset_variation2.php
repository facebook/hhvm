<?hh
/* Prototype  : mixed reset(&array $array_arg)
 * Description: Set array argument's internal pointer to the first element and return it
 * Source code: ext/standard/array.c
 */

/*
 * Unset first element of an array and test behaviour of reset()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing reset() : usage variations ***\n";

$array = varray['a', 'b', 'c'];

echo "\n-- Initial Position: --\n";
echo current($array) . " => " . key($array) . "\n";

echo "\n-- Unset First element in array and check reset() --\n";
unset($array[0]);
var_dump(reset(inout $array));
echo "===DONE===\n";
}
