<?hh
/* Prototype  : bool ctype_alnum(mixed $c)
 * Description: Checks for alphanumeric character(s) 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass incorrect number of arguments to ctype_alnum() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_alnum() : error conditions ***\n";

$orig = setlocale(LC_CTYPE, "C"); 

// Zero arguments
echo "\n-- Testing ctype_alnum() function with Zero arguments --\n";
try { var_dump( ctype_alnum() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_alnum with one more than the expected number of arguments
echo "\n-- Testing ctype_alnum() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
try { var_dump( ctype_alnum($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
