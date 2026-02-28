<?hh
/* Prototype  : array array_diff_uassoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Computes the difference of arrays with additional index check which is performed by a
 *                 user supplied callback function
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_uassoc() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = dict[0 => '0', 10 => '10', -10 => '-10', 20 =>'20', -20 => '-20'];
$float_indx_array = dict[0 => '0.0', 10 => '10.5', -10 => '-10.5', 0 => '0.5'];

echo "\n-- Testing array_diff_key() function with float indexed array --\n";
var_dump( array_diff_uassoc($input_array, $float_indx_array, ($a, $b) ==> strcasecmp((string)$a, (string)$b)));
var_dump( array_diff_uassoc($float_indx_array, $input_array, ($a, $b) ==> strcasecmp((string)$a, (string)$b)));
echo "===DONE===\n";
}
