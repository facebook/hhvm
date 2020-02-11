<?hh
/* Prototype  : mixed end(array $array_arg)
 * Description: Advances array argument's internal pointer to the last element and return it
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to end() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing end() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing end() function with Zero arguments --\n";
try { var_dump( end() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test end with one more than the expected number of arguments
echo "\n-- Testing end() function with more than expected no. of arguments --\n";
$array_arg = varray[1, 2];
$extra_arg = 10;
try { var_dump( end(inout $array_arg, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
