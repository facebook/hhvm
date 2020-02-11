<?hh
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge_recursive() : associative arrays ***\n";

// Initialise the arrays
$arr1 = darray[1 => "one", 2 => varray[1, 2]];
$arr2 = darray[2 => 'three', "four" => varray["hello", 'world']];
$arr3 = darray[1 => varray[6, 7], 'four' => varray["str1", 'str2']];

// Calling array_merge_recursive() with default arguments
echo "-- With default argument --\n";
var_dump( array_merge_recursive($arr1) );

// Calling array_merge_recursive() with more arguments
echo "-- With more arguments --\n";
var_dump( array_merge_recursive($arr1,$arr2) );
var_dump( array_merge_recursive($arr1,$arr2,$arr3) );

echo "Done";
}
