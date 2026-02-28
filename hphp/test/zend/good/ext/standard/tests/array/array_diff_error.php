<?hh
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are 
 * not present in any of the others arguments. 
 * Source code: ext/standard/array.c
 */

/*
 * Test array_diff with less than the expected number of arguments 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff() : error conditions ***\n";
// Zero arguments
echo "\n-- Testing array_diff() function with zero arguments --\n";
try { var_dump( array_diff() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


// Testing array_diff with one less than the expected number of arguments
echo "\n-- Testing array_diff() function with less than expected no. of arguments --\n";
$arr1 = vec[1, 2];
try { var_dump( array_diff($arr1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
