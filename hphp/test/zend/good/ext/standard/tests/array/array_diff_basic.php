<?hh
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are not present 
 * in any of the others arguments. 
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of array_diff
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff() : basic functionality ***\n";

//Test indexed array with integers as elements
$array_int1 = vec[1, 2, 3, 4];
$array_int2 = vec[3, 4, 5, 6];

echo "-- Test indexed array with integers as elements --\n";
var_dump(array_diff($array_int1, $array_int2));
var_dump(array_diff($array_int2, $array_int1));


//Test indexed array with strings as elements
$array_string1 = vec['one', 'two', 'three', 'four'];
$array_string2 = vec['three', 'four', 'five', 'six'];

echo "-- Test indexed array with strings as elements --\n";
var_dump(array_diff($array_string1, $array_string2));
var_dump(array_diff($array_string2, $array_string1));

//Test associative array with strings as keys and integers as elements
$array_assoc_int1 = dict['one' => 1, 'two' => 2, 'three' => 3, 'four' => 4];
$array_assoc_int2 = dict['three' => 3, 'four' => 4, 'five' => 5, 'six' => 6];

echo "-- Test associative array with strings as keys and integers as elements --\n";
var_dump(array_diff($array_assoc_int1, $array_assoc_int2));
var_dump(array_diff($array_assoc_int2, $array_assoc_int1));

//Test associative array with strings as keys and elements
$array_assoc_str1 = dict['one' => 'un', 'two' => 'deux', 'three' => 'trois', 'four' => 'quatre'];
$array_assoc_str2 = dict['three' => 'trois', 'four' => 'quatre', 'five' => 'cinq', 'six' => 'six'];

echo "-- Test associative array with strings as keys and integers as elements --\n";
var_dump(array_diff($array_assoc_str1, $array_assoc_str2));
var_dump(array_diff($array_assoc_str2, $array_assoc_str1));

echo "-- Test array_diff with more than 2 arguments --\n";
var_dump(array_diff($array_int1, $array_int2, $array_string1, $array_string2));

echo "Done";
}
