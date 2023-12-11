<?hh
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_merge_recursive() by passing
 * array having duplicate keys.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge_recursive() : array with duplicate keys for \$arr1 argument ***\n";

/* initialize the array having duplicate keys */
// array with numeric keys
$arr1_numeric_key = dict[ 1 => "one", 2 => "two", 2 => vec[1, 2], 3 => "three", 1 => vec["duplicate", 'strings']];
// array with string keys
$arr1_string_key = dict["str1" => "hello", "str2" => 111, "str1" => "world", "str2" => 111.111];

// initialize the second argument
$arr2 = dict[0 => "one", "str1" => "two", 1 => vec["one", "two"]];

echo "-- With default argument --\n";
var_dump( array_merge_recursive($arr1_numeric_key) );
var_dump( array_merge_recursive($arr1_string_key) );

echo "-- With more arguments --\n";
var_dump( array_merge_recursive($arr1_numeric_key, $arr2) );
var_dump( array_merge_recursive($arr1_string_key, $arr2) );

echo "Done";
}
