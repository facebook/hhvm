<?hh
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are not
 * present in any of the others arguments.
 * Source code: ext/standard/array.c
 */

/*
 * Test that array_diff behaves as expected for comparing:
 * 1. the order of the array
 * 2. duplicate values
 * 3. duplicate key names
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff() : usage variations ***\n";

$array_index = darray [0 => 'a', 1 => 'b', 2 => 'c', 0 => 'd', 3 => 'b'];   //duplicate key (0), duplicate value (b)
$array_assoc = darray ['2' => 'c',   //same key=>value pair, different order
                      '1' => 'b',
                      '0' => 'a',
                      'b' => '3',   //key and value from array_index swapped
                      'c' => 2];    //same as above, using integer

var_dump(array_diff($array_index, $array_assoc));
var_dump(array_diff($array_assoc, $array_index));

echo "Done";
}
