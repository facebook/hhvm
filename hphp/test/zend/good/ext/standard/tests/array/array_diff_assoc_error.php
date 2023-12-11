<?hh
/* Prototype  : array array_diff_assoc(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of arr1 that have values which are not present 
 * in any of the others arguments but do additional checks whether the keys are equal 
 * Source code: ext/standard/array.c
 */

/*
 * Test errors for array_diff with too few\zero arguments
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_assoc() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_diff_assoc() function with zero arguments --\n";
try { var_dump( array_diff_assoc() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing array_diff_assoc with one less than the expected number of arguments
echo "\n-- Testing array_diff_assoc() function with less than expected no. of arguments --\n";
$arr1 = vec[1, 2];
try { var_dump( array_diff_assoc($arr1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


echo "Done";
}
