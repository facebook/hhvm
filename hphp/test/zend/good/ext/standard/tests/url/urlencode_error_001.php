<?hh
/* Prototype  : proto string urlencode(string str)
 * Description: URL-encodes string 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

// NB: basic functionality tested in tests/strings/001.phpt
<<__EntryPoint>> function main(): void {
echo "*** Testing urlencode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing urlencode() function with Zero arguments --\n";
try { var_dump( urlencode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test urlencode with one more than the expected number of arguments
echo "\n-- Testing urlencode() function with more than expected no. of arguments --\n";
$str = 'string_val';
$extra_arg = 10;
try { var_dump( urlencode($str, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
