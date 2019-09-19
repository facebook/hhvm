<?hh
/* Prototype  : mixed array_shift(array &$stack)
 * Description: Pops an element off the beginning of the array
 * Source code: ext/standard/array.c
 */

/*
 * Test that the internal pointer is reset after calling array_shift()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_shift() : usage variations ***\n";

$stack = array ('one' => 'un', 'two' => 'deux');

echo "\n-- Call array_shift() --\n";
var_dump($result = array_shift(&$stack));

echo "\n-- Position of Internal Pointer in Passed Array: --\n";
echo key($stack) . " => " . current($stack) . "\n";

echo "Done";
}
