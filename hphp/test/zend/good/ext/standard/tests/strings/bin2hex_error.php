<?hh

/* Prototype  : string bin2hex  ( string $str  )
 * Description: Convert binary data into hexadecimal representation
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing bin2hex() : error conditions ***\n";

echo "\n-- Testing bin2hex() function with no arguments --\n";
try { var_dump( bin2hex() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing bin2hex() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( bin2hex("Hello World", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
