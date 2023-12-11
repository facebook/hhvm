<?hh
/* Prototype  : mixed array_shift(&array &$stack)
 * Description: Pops an element off the beginning of the array
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of array_shift()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_shift() : basic functionality ***\n";

$array = dict[0 => 'zero', 1 => 'one', '3' => 'three', 'four' => 4];
echo "\n-- Before shift: --\n";
var_dump($array);

echo "\n-- After shift: --\n";
echo "Returned value:\t";
var_dump(array_shift(inout $array));
echo "New array:\n";
var_dump($array);

echo "Done";
}
