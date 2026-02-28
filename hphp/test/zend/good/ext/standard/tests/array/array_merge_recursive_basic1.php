<?hh
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge_recursive() : array with default keys ***\n";

// Initialise the arrays
$arr1 = vec[1, vec[1, 2]];
$arr2 = vec[3, vec["hello", 'world']];
$arr3 = vec[vec[6, 7], vec["str1", 'str2']];

// Calling array_merge_recursive() with default arguments
echo "-- With default argument --\n";
var_dump( array_merge_recursive($arr1) );

// Calling array_merge_recursive() with more arguments
echo "-- With more arguments --\n";
var_dump( array_merge_recursive($arr1,$arr2) );
var_dump( array_merge_recursive($arr1,$arr2,$arr3) );

echo "Done";
}
