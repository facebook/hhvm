<?hh
/* Prototype  : array array_diff_uassoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Computes the difference of arrays with additional index check which is performed by a
 *                 user supplied callback function
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_uassoc() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = dict[0 => '0', 1 => '1', -10 => '-10', 'true' => 1, 'false' => 0];
$boolean_indx_array = dict[1 => 'boolt', 0 => 'boolf', 1 => 'boolT', 0 => 'boolF'];

echo "\n-- Testing array_diff_key() function with float indexed array --\n";
try { var_dump( array_diff_uassoc($input_array, $boolean_indx_array, strcasecmp<>) ); } catch (Exception $e) { var_dump($e->getMessage()); }
try { var_dump( array_diff_uassoc($boolean_indx_array, $input_array, strcasecmp<>) ); } catch (Exception $e) { var_dump($e->getMessage()); }
echo "===DONE===\n";
}
