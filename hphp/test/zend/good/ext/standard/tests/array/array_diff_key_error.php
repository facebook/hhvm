<?hh
/* Prototype  : array array_diff_key(array arr1, array arr2 [, array ...])
 * Description: Returns the entries of arr1 that have keys which are not present in any of the others arguments. 
 * Source code: ext/standard/array.c
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_key() : error conditions ***\n";

// Initialise the variables
$array1 = dict['blue' => 1, 'red' => 2, 'green' => 3, 'purple' => 4];

// Testing array_diff_key with one less than the expected number of arguments
echo "\n-- Testing array_diff_key() function with less than expected no. of arguments --\n";
try { var_dump( array_diff_key($array1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing array_diff_key with no arguments
echo "\n-- Testing array_diff_key() function with no arguments --\n";
try { var_dump( array_diff_key() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
