<?hh
/* Prototype  : mixed next(array $array_arg)
 * Description: Move array argument's internal pointer to the next element and return it
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of next()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing next() : basic functionality ***\n";

$array = varray['zero', 'one', 'two'];
echo key($array) . " => " . current($array) . "\n";
var_dump(next(inout $array));

echo key($array) . " => " . current($array) . "\n";
var_dump(next(inout $array));

echo key($array) . " => " . current($array) . "\n";
var_dump(next(inout $array));
echo "===DONE===\n";
}
