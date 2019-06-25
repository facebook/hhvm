<?hh
/* Prototype  : bool ctype_upper(mixed $c)
 * Description: Checks for uppercase character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass incorrect number of arguments to ctype_upper() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_upper() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing ctype_upper() function with Zero arguments --\n";
try { var_dump( ctype_upper() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_upper with one more than the expected number of arguments
echo "\n-- Testing ctype_upper() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
try { var_dump( ctype_upper($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
