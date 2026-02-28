<?hh
/* Prototype  : array array_diff_ukey(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments.
 * Source code: ext/standard/array.c
 */

function key_compare_func($key1, $key2)
:mixed{
  return strcasecmp((string)$key1, (string)$key2);
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_ukey() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = dict[0 => '0', 10 => '10', -10 => '-10', 20 =>'20'];
$float_indx_array = dict[0 => '0.0', 10 => '10.5', -30 => '-30.5'];

echo "\n-- Testing array_diff_ukey() function with float indexed array --\n";

var_dump( array_diff_ukey($float_indx_array, $input_array, key_compare_func<>) );
var_dump( array_diff_ukey($input_array, $float_indx_array, key_compare_func<>) );

echo "===DONE===\n";
}
