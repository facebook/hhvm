<?hh
/* Prototype  : mixed reset(&array $array_arg)
 * Description: Set array argument's internal pointer to the first element and return it
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of reset()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing reset() : basic functionality ***\n";

$array = darray[0 => 'zero', 1 => 'one', 200 => 'two'];

echo "\n-- Initial Position: --\n";
echo key($array) . " => " . current($array) . "\n";

echo "\n-- Call to next() --\n";
var_dump(next(inout $array));

echo "\n-- Current Position: --\n";
echo key($array) . " => " . current($array) . "\n";

echo "\n-- Call to reset() --\n";
var_dump(reset(inout $array));
echo "===DONE===\n";
}
