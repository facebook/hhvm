<?hh
/* Prototype  : mixed key(array $array_arg)
 * Description: Return the key of the element currently pointed to by the internal array pointer
 * Source code: ext/standard/array.c
 */

/*
 * Pass incorrect number of arguments to key() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing key() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing key() function with Zero arguments --\n";
try { var_dump( key() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test current with one more than the expected number of arguments
echo "\n-- Testing key() function with more than expected no. of arguments --\n";
$array_arg = varray[1, 2];
$extra_arg = 10;
try { var_dump( key($array_arg, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
