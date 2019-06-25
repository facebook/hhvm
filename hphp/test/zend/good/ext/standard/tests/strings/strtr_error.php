<?hh
/* Prototype  : string strtr(string str, string from[, string to])
 * Description: Translates characters in str using given translation tables 
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strtr() : error conditions ***\n";
$str = "string";
$from = "string";
$to = "STRING";
$extra_arg = "extra_argument";

echo "\n-- Testing strtr() function with Zero arguments --";
try { var_dump( strtr() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strtr() function with less than expected no. of arguments --";
try { var_dump( strtr($str) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strtr() function with more than expected no. of arguments --";
try { var_dump( strtr($str, $from, $to, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
