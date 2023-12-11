<?hh
/* Prototype  : array array_udiff_assoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal. Keys are compared by user supplied function.
 * Source code: ext/standard/array.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
include('compare_function.inc');
echo "*** Testing array_udiff_assoc() : variation - testing with multiple array arguments ***\n";

$key_compare_function = compare_function<>;

// Initialise all required variables
$arr1 = dict["one" => "one", "02" => "two", '3' => "three", 0 => "four", "0.5" => 5, 6 => 6, "seven" => "0x7"];
$arr2 = dict["one" => "one", "02" => "two", '3' => "three"];
$arr3 = dict[0 => "four", "0.5" => "five", 6 => 6, "seven" => 7];
$arr4 = dict[0 => "four", "0.5" => "five", 6 => 6, "seven" => 7];


var_dump( array_udiff_assoc($arr1, $arr2, $arr3, $arr4, $key_compare_function) );


echo "===DONE===\n";
}
