<?hh
/* Prototype  : array array_udiff_assoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Returns the entries of arr1 that have values which are not present in any of the others arguments but do additional checks whether the keys are equal. Keys are compared by user supplied function.
 * Source code: ext/standard/array.c
 * Alias to functions:
 */
include('compare_function.inc');
<<__EntryPoint>> function main(): void {
echo "*** Testing array_udiff_assoc() : variation - testing with multiple array arguments ***\n";

$key_compare_function = 'compare_function';

// Initialise all required variables
$arr1 = array("one" => "one", "02" => "two", '3' => "three", "four", "0.5" => 5, 6.0 => 6, "seven" => "0x7");
$arr2 = array("one" => "one", "02" => "two", '3' => "three");
$arr3 = array("four", "0.5" => "five", 6 => 6, "seven" => 7);
$arr4 = array("four", "0.5" => "five", 6 => 6, "seven" => 7);


var_dump( array_udiff_assoc($arr1, $arr2, $arr3, $arr4, $key_compare_function) );


echo "===DONE===\n";
}
