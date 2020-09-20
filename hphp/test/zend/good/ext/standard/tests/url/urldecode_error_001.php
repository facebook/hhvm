<?hh
/* Prototype  : proto string urldecode(string str)
 * Description: Decodes URL-encoded string 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

// NB: basic functionality tested in tests/strings/001.phpt
<<__EntryPoint>> function main(): void {
echo "*** Testing urldecode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing urldecode() function with Zero arguments --\n";
try { var_dump( urldecode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test urldecode with one more than the expected number of arguments
echo "\n-- Testing urldecode() function with more than expected no. of arguments --\n";
$str = 'string_val';
$extra_arg = 10;
try { var_dump( urldecode($str, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
