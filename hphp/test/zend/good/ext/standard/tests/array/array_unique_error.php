<?hh
/* Prototype  : array array_unique(array $input)
 * Description: Removes duplicate values from array 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_unique() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_unique() function with zero arguments --\n";
try { var_dump( array_unique() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test array_unique with one more than the expected number of arguments
echo "\n-- Testing array_unique() function with more than expected no. of arguments --\n";
$input = vec[1, 2];
$extra_arg = 10;
try { var_dump( array_unique($input, SORT_NUMERIC, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
