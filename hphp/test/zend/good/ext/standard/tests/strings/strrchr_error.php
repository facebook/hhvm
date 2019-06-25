<?hh
/* Prototype  : string strrchr(string $haystack, string $needle);
 * Description: Finds the last occurrence of a character in a string. 
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strrchr() function: error conditions ***\n";
$haystack = "Hello";
$needle = "Hello";
$extra_arg = "Hello";

echo "\n-- Testing strrchr() function with Zero arguments --";
try { var_dump( strrchr() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strrchr() function with less than expected no. of arguments --";
try { var_dump( strrchr($haystack) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing strrchr() function with more than expected no. of arguments --";
try { var_dump( strrchr($haystack, $needle, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "*** Done ***";
}
