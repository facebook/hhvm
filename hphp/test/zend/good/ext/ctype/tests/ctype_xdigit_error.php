<?hh
/* Prototype  : bool ctype_xdigit(mixed $c)
 * Description: Checks for character(s) representing a hexadecimal digit 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass incorrect number of arguments to ctype_xdigit() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_xdigit() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing ctype_xdigit() function with Zero arguments --\n";
try { var_dump( ctype_xdigit() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_xdigit with one more than the expected number of arguments
echo "\n-- Testing ctype_xdigit() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
try { var_dump( ctype_xdigit($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
