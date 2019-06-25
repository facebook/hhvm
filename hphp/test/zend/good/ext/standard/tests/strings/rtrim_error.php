<?hh

/* Prototype  : string rtrim  ( string $str  [, string $charlist  ] )
 * Description: Strip whitespace (or other characters) from the end of a string.
 * Source code: ext/standard/string.c
*/

<<__EntryPoint>> function main(): void {
echo "*** Testing rtrim() : error conditions ***\n";

echo "\n-- Testing rtrim() function with no arguments --\n";
try { var_dump( rtrim() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing rtrim() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( rtrim("Hello World",  "Heo", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


$hello = "  Hello World\n";
echo "\n-- Test rtrim function with various invalid charlists\n";
var_dump(rtrim($hello, "..a"));
var_dump(rtrim($hello, "a.."));
var_dump(rtrim($hello, "z..a"));
var_dump(rtrim($hello, "a..b..c"));

echo "===DONE===\n";
}
