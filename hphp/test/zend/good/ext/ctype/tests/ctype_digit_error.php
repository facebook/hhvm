<?hh
/* Prototype  : bool ctype_digit(mixed $c)
 * Description: Checks for numeric character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass an incorrect number of arguments to ctype_digit() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_digit() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing ctype_digit() function with Zero arguments --\n";
try { var_dump( ctype_digit() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_digit with one more than the expected number of arguments
echo "\n-- Testing ctype_digit() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
try { var_dump( ctype_digit($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
