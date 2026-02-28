<?hh
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are not
 * present in any of the others arguments.
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff compares arrays that
 * 1. Contain referenced variables
 * 2. Have been referenced to each other
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff() : usage variations ***\n";
$a = 'a';

$arr1 = vec["&$a", 'b', 'c'];
$arr2 = vec[1, 2, 3];
echo "-- Basic Comparison --\n";
var_dump(array_diff($arr1, $arr2));
var_dump(array_diff($arr2, $arr1));

$a = 1;

echo "-- \$a changed --\n";
var_dump(array_diff($arr1, $arr2));
var_dump(array_diff($arr2, $arr1));

echo "Done";
}
