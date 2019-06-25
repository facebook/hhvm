<?hh
/* Prototype  : bool ctype_punct(mixed $c)
 * Description: Checks for any printable character which is not whitespace 
 * or an alphanumeric character 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass incorrect number of arguments to ctype_punct() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_punct() : error conditions ***\n";

$orig = setlocale(LC_CTYPE, "C"); 

// Zero arguments
echo "\n-- Testing ctype_punct() function with Zero arguments --\n";
try { var_dump( ctype_punct() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_punct with one more than the expected number of arguments
echo "\n-- Testing ctype_punct() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
try { var_dump( ctype_punct($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

setlocale(LC_CTYPE, $orig); 
echo "===DONE===\n";
}
