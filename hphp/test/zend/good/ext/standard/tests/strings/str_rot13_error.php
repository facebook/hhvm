<?hh
/* Prototype  : string str_rot13  ( string $str  )
 * Description: Perform the rot13 transform on a string
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing str_rot13() : error conditions ***\n";
echo "-- Testing str_rot13() function with Zero arguments --\n";
try { var_dump( str_rot13() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n\n-- Testing str_rot13() function with more than expected no. of arguments --\n";
$str = "str_rot13() tests starting";
$extra_arg = 10;
try { var_dump( str_rot13( $str, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
