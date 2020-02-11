<?hh
/* Prototype  : mixed reset(&array $array_arg)
 * Description: Set array argument's internal pointer to the first element and return it
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to reset() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing reset() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing reset() function with Zero arguments --\n";
try { var_dump( reset() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test reset with one more than the expected number of arguments
echo "\n-- Testing reset() function with more than expected no. of arguments --\n";
$array_arg = varray[1, 2];
$extra_arg = 10;
try { var_dump( reset(inout $array_arg, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
