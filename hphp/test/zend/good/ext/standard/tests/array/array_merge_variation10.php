<?hh
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
 */

/*
 * Check the position of the internal array pointer after calling array_merge().
 * This test is also passing more than two arguments to array_merge().
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge() : usage variations ***\n";

$arr1 = varray ['zero', 'one', 'two'];
$arr2 = varray ['zero', 'un', 'deux'];
$arr3 = varray ['null', 'eins', 'zwei'];

echo "\n-- Call array_merge() --\n";
var_dump($result = array_merge($arr1, $arr2, $arr3));
echo "Done";
}
