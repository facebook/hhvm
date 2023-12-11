<?hh
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_merge() with multi-dimensional arrays
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge() : usage variations ***\n";

$arr1 = vec['zero', 'one', 'two', vec[0]];
$arr2 = vec[1, 2, 3];

echo "\n-- Merge a two-dimensional and a one-dimensional array --\n";
var_dump(array_merge($arr1, $arr2));

echo "\n-- Merge an array and a sub-array --\n";
var_dump(array_merge($arr1[3], $arr2));
var_dump(array_merge($arr2, $arr1[3]));

echo "Done";
}
