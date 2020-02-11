<?hh
/* Prototype  : mixed current(array $array_arg)
 * Description: Return the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 * Alias to functions: pos
 */

/*
 * Pass incorrect number of arguments to current() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing current() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing current() function with Zero arguments --\n";
try { var_dump( current() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test current with one more than the expected number of arguments
echo "\n-- Testing current() function with more than expected no. of arguments --\n";
$array_arg = varray[1, 2];
$extra_arg = 10;
try { var_dump( current($array_arg, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
