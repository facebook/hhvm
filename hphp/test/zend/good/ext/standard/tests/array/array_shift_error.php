<?hh
/* Prototype  : mixed array_shift(array &$stack)
 * Description: Pops an element off the beginning of the array
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to array_shift() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_shift() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing array_shift() function with Zero arguments --\n";
try { var_dump( array_shift() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test array_shift with one more than the expected number of arguments
echo "\n-- Testing array_shift() function with more than expected no. of arguments --\n";
$stack = vec[1, 2];
$extra_arg = 10;
try { var_dump( array_shift(inout $stack, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
