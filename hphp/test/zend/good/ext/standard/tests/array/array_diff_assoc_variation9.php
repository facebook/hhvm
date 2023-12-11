<?hh
/* Prototype  : array array_diff_assoc(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are not 
 * present in any of the others arguments but do additional checks whether 
 * the keys are equal 
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff_assoc behaves when comparing 
 * multi-dimensional arrays
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_assoc() : usage variations ***\n";

$array1 = dict['sub_array1' => varray [1, 2, 3],
                'sub_array2' => varray ['a', 'b', 'c']];
$array2 = dict['sub_arraya' => varray [1, 3, 5],
                'sub_arrayb' => varray ['a', 'z', 'y']];

echo "-- Compare two 2-D arrays --\n";
var_dump(array_diff_assoc($array1, $array2));
var_dump(array_diff_assoc($array2, $array1));
     
echo "\n-- Compare subarrays from two 2-D arrays --\n";
var_dump(array_diff_assoc($array1['sub_array1'], $array2['sub_arraya']));
var_dump(array_diff_assoc($array2['sub_arraya'], $array1['sub_array1']));
var_dump(array_diff_assoc($array1['sub_array2'], $array2['sub_arrayb']));
var_dump(array_diff_assoc($array2['sub_arrayb'], $array1['sub_array1']));

echo "\n-- Compare a subarray from one 2-D array and one 2-D array --\n";
var_dump(array_diff_assoc($array1['sub_array1'], $array2));
var_dump(array_diff_assoc($array1, $array2['sub_arraya']));

echo "Done";
}
