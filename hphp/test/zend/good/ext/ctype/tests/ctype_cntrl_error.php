<?hh
/* Prototype  : bool ctype_cntrl(mixed $c)
 * Description: Checks for control character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass an incorrect number of arguments to ctype_cntrl() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_cntrl() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing ctype_cntrl() function with Zero arguments --\n";
try { var_dump( ctype_cntrl() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_cntrl with one more than the expected number of arguments
echo "\n-- Testing ctype_cntrl() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
try { var_dump( ctype_cntrl($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
