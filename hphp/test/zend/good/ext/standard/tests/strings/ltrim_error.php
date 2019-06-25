<?hh

/* Prototype  : string ltrim  ( string $str  [, string $charlist  ] )
 * Description: Strip whitespace (or other characters) from the beginning of a string.
 * Source code: ext/standard/string.c
*/

<<__EntryPoint>> function main(): void {
echo "*** Testing ltrim() : error conditions ***\n";

echo "\n-- Testing ltrim() function with no arguments --\n";
try { var_dump( ltrim() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing ltrim() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( ltrim("Hello World",  "Heo", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


$hello = "  Hello World\n";
echo "\n-- Test ltrim function with various invalid charlists\n";
var_dump(ltrim($hello, "..a"));
var_dump(ltrim($hello, "a.."));
var_dump(ltrim($hello, "z..a"));
var_dump(ltrim($hello, "a..b..c"));

echo "===DONE===\n";
}
