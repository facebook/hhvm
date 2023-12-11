<?hh
/* Prototype  : array array_values(array $input)
 * Description: Return just the values from the input array 
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to array_values to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_values() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_values() function with Zero arguments --\n";
try { var_dump( array_values() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test array_values with one more than the expected number of arguments
echo "\n-- Testing array_values() function with more than expected no. of arguments --\n";
$input = vec[1, 2];
$extra_arg = 10;
try { var_dump( array_values($input, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
