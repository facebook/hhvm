<?hh
/* Prototype  : string nl2br(string $str)
 * Description: Inserts HTML line breaks before all newlines in a string.
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing nl2br() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing nl2br() function with Zero arguments --";
try { var_dump( nl2br() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test nl2br with one more than the expected number of arguments
echo "\n-- Testing nl2br() function with more than expected no. of arguments --";
$str = 'string_val';
$extra_arg = 10;
try { var_dump( nl2br($str, true, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
