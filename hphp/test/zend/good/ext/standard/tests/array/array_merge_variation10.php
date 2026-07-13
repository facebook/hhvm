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

$arr1 = vec['zero', 'one', 'two'];
$arr2 = vec['zero', 'un', 'deux'];
$arr3 = vec['null', 'eins', 'zwei'];

echo "\n-- Call array_merge() --\n";
$result = array_merge($arr1, $arr2, $arr3);
var_dump($result);
echo "Done";
}
