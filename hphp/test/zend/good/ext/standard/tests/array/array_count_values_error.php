<?hh
/* Prototype  : proto array array_count_values(array input)
 * Description: Return the value as key and the frequency of that value in input as value 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */

/*
 * Test for handling of incorrect parameters.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_count_values() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_count_values() function with Zero arguments --\n";
try { var_dump( array_count_values() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test array_count_values with one more than the expected number of arguments
echo "\n-- Testing array_count_values() function with more than expected no. of arguments --\n";
$input = vec[1, 2];
$extra_arg = 10;
try { var_dump( array_count_values($input, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test array_count_values with integer arguments
echo "\n-- Testing array_count_values() function integer arguments --\n";
var_dump( array_count_values(1 ));

echo "Done";
}
