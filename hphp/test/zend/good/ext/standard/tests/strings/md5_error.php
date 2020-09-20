<?hh
/* Prototype  : string md5  ( string $str  [, bool $raw_output= false  ] )
 * Description: Calculate the md5 hash of a string
 * Source code: ext/standard/md5.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing md5() : error conditions ***\n";

echo "\n-- Testing md5() function with no arguments --\n";
try { var_dump( md5()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing md5() function with more than expected no. of arguments --\n";
$str = "Hello World";
$raw_output = true;
$extra_arg = 10;

try { var_dump(md5($str, $raw_output, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE==";
}
