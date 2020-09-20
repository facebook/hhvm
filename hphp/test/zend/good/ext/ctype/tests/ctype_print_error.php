<?hh
/* Prototype  : bool ctype_print(mixed $c)
 * Description: Checks for printable character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass incorrect number of arguments to ctype_print() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_print() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing ctype_print() function with Zero arguments --\n";
try { var_dump( ctype_print() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_print with one more than the expected number of arguments
echo "\n-- Testing ctype_print() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
try { var_dump( ctype_print($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
