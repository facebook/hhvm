<?hh
/* Prototype  : mixed next(array $array_arg)
 * Description: Move array argument's internal pointer to the next element and return it
 * Source code: ext/standard/array.c
 */

/*
 * Test next() when passed:
 * 1. a two-dimensional array
 * 2. a sub-array
 * as $array_arg argument.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing next() : usage variations ***\n";

$array_arg = darray['a' => 'z', 0 => varray[9, 8, 7]];

echo "\n-- Pass a two-dimensional array as \$array_arg --\n";
var_dump(next(inout $array_arg));
var_dump(next(inout $array_arg));

echo "===DONE===\n";
}
