<?hh
/* Prototype  : bool ctype_alpha(mixed $c)
 * Description: Checks for alphabetic character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass an incorrect number of arguments to ctype_alpha() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_alpha() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing ctype_alpha() function with Zero arguments --\n";
try { var_dump( ctype_alpha() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_alpha with one more than the expected number of arguments
echo "\n-- Testing ctype_alpha() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
try { var_dump( ctype_alpha($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
