<?hh
/* Prototype  : array array_merge(array $arr1, array $arr2 [, array $...])
 * Description: Merges elements from passed arrays into one array 
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of array_merge()
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge() : basic functionality ***\n";

//indexed array
$array1 = vec['zero', 'one', 'two'];
//associative array
$array2 = dict['a' => 1, 'b' => 2, 'c' => 3];

var_dump(array_merge($array1, $array2));

var_dump(array_merge($array2, $array1));

echo "Done";
}
