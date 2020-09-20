<?hh
/* Prototype  : bool ctype_space(mixed $c)
 * Description: Checks for whitespace character(s)
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass an incorrect number of arguments to ctype_space() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_space() : error conditions ***\n";

$orig = setlocale(LC_CTYPE, "C");

// Zero arguments
echo "\n-- Testing ctype_space() function with Zero arguments --\n";
try { var_dump( ctype_space() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_space with one more than the expected number of arguments
echo "\n-- Testing ctype_space() function with more than expected no. of arguments --\n";
$c = " ";
$extra_arg = 10;
try { var_dump( ctype_space($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

setlocale(LC_CTYPE, $orig);
echo "===DONE===\n";
}
