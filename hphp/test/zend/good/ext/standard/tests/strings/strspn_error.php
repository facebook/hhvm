<?hh
/* Prototype  : proto int strspn(string str, string mask [, int start [, int len]])
 * Description: Finds length of initial segment consisting entirely of characters found in mask.
                If start or/and length is provided works like strspn(substr($s,$start,$len),$good_chars)
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* Test strspn() : for error conditons
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strspn() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing strspn() function with Zero arguments --\n";
try { var_dump( strspn() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test strspn with one more than the expected number of arguments
echo "\n-- Testing strspn() function with more than expected no. of arguments --\n";
$str = 'string_val';
$mask = 'string_val';
$start = 2;
$len = 20;


$extra_arg = 10;
try { var_dump( strspn($str,$mask,$start,$len, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing strspn withone less than the expected number of arguments
echo "\n-- Testing strspn() function with less than expected no. of arguments --\n";
$str = 'string_val';
try { var_dump( strspn($str) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
