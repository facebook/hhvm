<?hh
/* Prototype  : string soundex  ( string $str  )
 * Description: Calculate the soundex key of a string
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "\n*** Testing soundex error conditions ***";

echo "-- Testing soundex() function with Zero arguments --\n";
try { var_dump( soundex() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n\n-- Testing soundex() function with more than expected no. of arguments --\n";
$str = "Euler";
$extra_arg = 10;
try { var_dump( soundex( $str, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
