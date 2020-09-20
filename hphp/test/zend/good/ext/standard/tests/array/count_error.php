<?hh
/* Prototype  : int count(mixed var [, int mode])
 * Description: Count the number of elements in a variable (usually an array) 
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to count() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing count() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing count() function with Zero arguments --\n";
try { var_dump( count() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test count with one more than the expected number of arguments
echo "\n-- Testing count() function with more than expected no. of arguments --\n";
$var = 1;
$mode = 10;
$extra_arg = 10;
try { var_dump( count($var, $mode, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
