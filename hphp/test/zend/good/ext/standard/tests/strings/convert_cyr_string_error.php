<?hh

/* Prototype  : string convert_cyr_string  ( string $str  , string $from  , string $to  )
 * Description: Convert from one Cyrillic character set to another
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
$str = "hello";
$from = "k";
$to = "d";
$extra_arg = 10;

echo "*** Testing convert_cyr_string() : error conditions ***\n";

echo "\n-- Testing convert_cyr_string() function with no arguments --\n";
try { var_dump( convert_cyr_string() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing convert_cyr_string() function with no 'to' character set --\n";
try { var_dump( convert_cyr_string($str, $from) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing convert_cyr_string() function with more than expected no. of arguments --\n";
try { var_dump( convert_cyr_string($str, $from, $to, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing convert_cyr_string() function with invalid 'from' character set --\n";
var_dump(bin2hex( convert_cyr_string($str, "?", $to) ));

echo "\n-- Testing convert_cyr_string() function with invalid 'to' character set --\n";
var_dump(bin2hex( convert_cyr_string($str, $from, "?")) );

echo "\n-- Testing convert_cyr_string() function with invalid 'from' and 'to' character set --\n";
var_dump(bin2hex( convert_cyr_string($str, ">", "?")) );
echo "===DONE===\n";
}
