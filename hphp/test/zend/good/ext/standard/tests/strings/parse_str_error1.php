<?hh
/* Prototype  : void parse_str  ( string $str  [, array &$arr  ] )
 * Description: Parses the string into variables
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing parse_str() : error conditions ***\n";

echo "\n-- Testing htmlentities() function with less than expected no. of arguments --\n";
try { parse_str(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "\n-- Testing htmlentities() function with more than expected no. of arguments --\n";
$s1 = "first=val1&second=val2&third=val3";
$res_array = null;
try { parse_str($s1, inout $res_array, true); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
