<?hh
/* Prototype  : string money_format  ( string $format  , float $number  )
 * Description: Formats a number as a currency string
 * Source code: ext/standard/string.c
*/

// ===========================================================================================
// = We do not test for exact return-values, as those might be different between OS-versions =
// ===========================================================================================
<<__EntryPoint>> function main(): void {
$string = '%14#8.2n';
$value = 1234.56;
$extra_arg = 10;

echo "*** Testing money_format() : error conditions ***\n";

echo "\n-- Testing money_format() function with no arguments --\n";
try { var_dump( money_format() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing money_format() function with insufficient arguments --\n";
try { var_dump( money_format($string) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing money_format() function with more than expected no. of arguments --\n";

try { var_dump( money_format($string, $value, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
