<?hh
/* Prototype  : proto int strcspn(string str, string mask [, int start [, int len]])
 * Description: Finds length of initial segment consisting entirely of characters not found in mask.
                If start or/and length is provided works like strcspn(substr($s,$start,$len),$bad_chars)
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* Test strcspn() : for error conditons
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strcspn() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing strcspn() function with Zero arguments --\n";
try { var_dump( strcspn() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test strcspn with one more than the expected number of arguments
echo "\n-- Testing strcspn() function with more than expected no. of arguments --\n";
$str = 'string_val';
$mask = 'string_val';
$start = 2;
$len = 20;


$extra_arg = 10;
try { var_dump( strcspn($str,$mask,$start,$len, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing strcspn withone less than the expected number of arguments
echo "\n-- Testing strcspn() function with less than expected no. of arguments --\n";
$str = 'string_val';
try { var_dump( strcspn($str) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
