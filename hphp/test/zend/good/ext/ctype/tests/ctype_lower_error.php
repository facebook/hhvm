<?hh
/* Prototype  : bool ctype_lower(mixed $c)
 * Description: Checks for lowercase character(s)  
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass incorrect number of arguments to ctype_lower() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_lower() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing ctype_lower() function with Zero arguments --\n";
try { var_dump( ctype_lower() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_lower with one more than the expected number of arguments
echo "\n-- Testing ctype_lower() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
try { var_dump( ctype_lower($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
