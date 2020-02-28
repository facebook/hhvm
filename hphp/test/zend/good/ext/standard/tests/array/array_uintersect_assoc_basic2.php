<?hh
/* Prototype  : array array_uintersect_assoc(array arr1, array arr2 [, array ...], callback data_compare_func)
 * Description: U
 * Source code: ext/standard/array.c
 * Alias to functions:
 */
include('compare_function.inc');
<<__EntryPoint>> function main(): void {
echo "*** Testing array_uintersect_assoc() : basic functionality - testing with multiple array arguments ***\n";

$data_compare_function = 'compare_function';

// Initialise all required variables
$arr1 = darray["one" => "one", "02" => "two", '3' => "three", 0 => "four", "0.5" => 5, 0.6 => 6, "0x7" => "seven"];
$arr2 = darray["one" => "one", "02" => "two", '3' => "three"];
$arr3 = darray["one" => "one", '3' => "three", "0.5" => 5];
$arr4 = darray["one" => "one", '3' => "three", "0.5" => 5];


var_dump( array_uintersect_assoc($arr1, $arr2, $arr3, $arr4, $data_compare_function) );


echo "===DONE===\n";
}
