<?hh
/* Prototype  : mixed array_sum(array &input)
 * Description: Returns the sum of the array entries 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_sum() : error conditions ***\n";

// Zero arguments
echo "-- Testing array_sum() function with zero arguments --\n";
try { var_dump( array_sum() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// One more than the expected number of arguments
echo "-- Testing array_sum() function with more than expected no. of arguments --\n";
$input = vec[1, 2, 3, 4];
$extra_arg = 10;
try { var_dump( array_sum($input, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
