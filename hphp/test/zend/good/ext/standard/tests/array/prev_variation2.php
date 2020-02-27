<?hh
/* Prototype  : mixed prev(array $array_arg)
 * Description: Move array argument's internal pointer to the previous element and return it
 * Source code: ext/standard/array.c
 */

/*
 * Test prev() when passed:
 * 1. a two-dimensional array
 * 2. a sub-array
 * as $array_arg argument.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing prev() : usage variations ***\n";

$subarray = varray[9,8,7];
end(inout $subarray);

$array_arg = darray[0 => $subarray, 'a' => 'z'];
end(inout $array_arg);

echo "\n-- Pass a two-dimensional array as \$array_arg --\n";
var_dump(prev(inout $array_arg));
var_dump(prev(inout $array_arg));

echo "===DONE===\n";
}
