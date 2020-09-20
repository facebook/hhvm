<?hh
/* Prototype  : int strnatcmp  ( string $str1  , string $str2  )
 * Description: String comparisons using a "natural order" algorithm
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing strnatcmp() : error conditions ***\n";
echo "-- Testing strnatcmp() function with Zero arguments --\n";
try { var_dump( strnatcmp() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n\n-- Testing strnatcmp() function with more than expected no. of arguments --\n";
$str1 = "abc1";
$str2 = "ABC1";
$extra_arg = 10;
try { var_dump( strnatcmp( $str1, $str2, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
