<?hh
/* Prototype  : array array_diff_assoc(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of arr1 that have values which are not present
 * in any of the others arguments but do additional checks whether the keys are equal
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff_assoc compares integers, floats and string
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_assoc() : usage variations ***\n";
$arr_default_int = vec[1, 2, 3, 'a'];
$arr_float = dict[0 => 1.00, 1 => 2.00, 2 => 3.00, 3 => 'b'];
$arr_string = vec['1', '2', '3', 'c'];
$arr_string_float = dict['0' => '1.00', '1.00' => '2.00', '2.00' => '3.00', 0 => 'd'];

echo "-- Result of comparing integers and floating point numbers: --\n";
var_dump(array_diff_assoc($arr_default_int, $arr_float));
var_dump(array_diff_assoc($arr_float, $arr_default_int));

echo "-- Result of comparing integers and strings containing an integers : --\n";
var_dump(array_diff_assoc($arr_default_int, $arr_string));
var_dump(array_diff_assoc($arr_string, $arr_default_int));

echo "-- Result of comparing integers and strings containing floating points : --\n";
var_dump(array_diff_assoc($arr_default_int, $arr_string_float));
var_dump(array_diff_assoc($arr_string_float, $arr_default_int));

echo "-- Result of comparing floating points and strings containing integers : --\n";
var_dump(array_diff_assoc($arr_float, $arr_string));
var_dump(array_diff_assoc($arr_string, $arr_float));

echo "-- Result of comparing floating points and strings containing floating point: --\n";
var_dump(array_diff_assoc($arr_float, $arr_string_float));
var_dump(array_diff_assoc($arr_string_float, $arr_float));

echo "-- Result of comparing strings containing integers and strings containing floating points : --\n";
var_dump(array_diff_assoc($arr_string, $arr_string_float));
var_dump(array_diff_assoc($arr_string_float, $arr_string));

echo "-- Result of comparing more than two arrays: --\n";
var_dump(array_diff_assoc($arr_default_int, $arr_float, $arr_string, $arr_string_float));

echo "Done";
}
