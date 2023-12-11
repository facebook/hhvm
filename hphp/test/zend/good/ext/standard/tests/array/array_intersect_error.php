<?hh
/* Prototype  : array array_intersect(array $arr1, array $arr2 [, array $...])
 * Description: Returns the entries of arr1 that have values which are present in all the other arguments 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_intersect() : error conditions ***\n";

// Testing array_intersect() with zero arguments
echo "\n-- Testing array_intersect() function with Zero arguments --\n";
try { var_dump( array_intersect() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing array_intersect() with one less than the expected number of arguments
echo "\n-- Testing array_intersect() function with less than expected no. of arguments --\n";
$arr1 = vec[1, 2];
try { var_dump( array_intersect($arr1) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
