<?hh

/* Prototype  : string trim  ( string $str  [, string $charlist  ] )
 * Description: Strip whitespace (or other characters) from the beginning and end of a string.
 * Source code: ext/standard/string.c
*/

<<__EntryPoint>> function main(): void {
echo "*** Testing trim() : error conditions ***\n";

echo "\n-- Testing trim() function with no arguments --\n";
try { var_dump( trim() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing trim() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( trim("Hello World",  "Heo", $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }


$hello = "  Hello World\n";
echo "\n-- Test trim function with various invalid charlists --\n";
var_dump(trim($hello, "..a"));
var_dump(trim($hello, "a.."));
var_dump(trim($hello, "z..a"));
var_dump(trim($hello, "a..b..c"));

echo "===DONE===\n";
}
