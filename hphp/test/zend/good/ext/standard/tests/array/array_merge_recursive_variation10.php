<?hh
/* Prototype  : array array_merge_recursive(array $arr1[, array $...])
 * Description: Recursively merges elements from passed arrays into one array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_merge_recursive() by passing
 * two dimensional arrays for $arr1 argument.
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_merge_recursive() : two dimensional array for \$arr1 argument ***\n";

// initialize the 2-d array
$arr1 = dict[
  0 => vec[1, 2, 3, 1],
  "array" => dict[0 => "hello", 1 => "world", "str1" => "hello", "str2" => 'world'],
  1 => dict[1 => "one", 2 => "two", 3 => "one", 4 => 'two'],
  2 => vec[1, 2, 3, 1]
];

// initialize the second argument
$arr2 = dict[0 => 1, 1 => "hello", "array" => vec["hello", 'world']];

echo "-- Passing the entire 2-d array --\n";
echo "-- With default argument --\n";
var_dump( array_merge_recursive($arr1) );
echo "-- With more arguments --\n";
var_dump( array_merge_recursive($arr1, $arr2) );

echo "-- Passing the sub-array --\n";
echo "-- With default argument --\n";
var_dump( array_merge_recursive($arr1["array"]) );
echo "-- With more arguments --\n";
var_dump( array_merge_recursive($arr1["array"], $arr2["array"]) );

echo "Done";
}
