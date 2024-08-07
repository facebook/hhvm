<?hh
/* Prototype  : array array_diff_assoc(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are not
 * present in any of the others arguments but do additional checks whether
 * the keys are equal
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff_assoc() behaves when comparing:
 * 1. the order of the array
 * 2. duplicate values
 * 3. duplicate key names
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_assoc() : variation ***\n";

$array_index = dict[0 => 'a', 1 => 'b', 2 => 'c', 0 => 'd', 3 => 'b'];   //duplicate key (0), duplicate value (b)
$array_assoc = dict['2' => 'c',   //same key=>value pair, different order
                      '1' => 'b',
                      '0' => 'a',
                      'b' => '3',   //key and value from array_index swapped
                      'c' => 2];    //same as above, using integer

var_dump(array_diff_assoc($array_index, $array_assoc));
var_dump(array_diff_assoc($array_assoc, $array_index));

echo "Done";
}
