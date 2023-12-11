<?hh
/* Prototype  : bool usort(&array $array_arg, string $cmp_function)
 * Description: Sort an array by values using a user-defined comparison function
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to usort() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing usort() : error conditions ***\n";

//Test usort with one more than the expected number of arguments
echo "\n-- Testing usort() function with more than expected no. of arguments --\n";
$array_arg = vec[1, 2];
$cmp_function = 'string_val';
$extra_arg = 10;
try { var_dump( usort(inout $array_arg, $cmp_function, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing usort with one less than the expected number of arguments
echo "\n-- Testing usort() function with less than expected no. of arguments --\n";
$array_arg = vec[1, 2];
try { var_dump( usort(inout $array_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
