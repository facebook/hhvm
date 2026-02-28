<?hh
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_merge_recursive() by passing an array having binary values.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge_recursive() : array with binary data for \$arr1 argument ***\n";

// array with binary values
$arr1 = dict[0 => b"1", b"hello" => "hello", 1 => b"world", "str1" => b"hello", "str2" => "world"];

// initialize the second argument
$arr2 = dict[b"str1" => b"binary", b"hello" => "binary", b"str2" => b"binary"];

echo "-- With default argument --\n";
var_dump( array_merge_recursive($arr1) );

echo "-- With more arguments --\n";
var_dump( array_merge_recursive($arr1, $arr2) );

echo "Done";
}
