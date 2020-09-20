<?hh
/* Prototype  : proto string mb_ereg_replace(string pattern, string replacement, string string [, string option])
 * Description: Replace regular expression for multibyte string 
 * Source code: ext/mbstring/php_mbregex.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_ereg_replace() : error conditions ***\n";

//Test mb_ereg_replace with one more than the expected number of arguments
echo "\n-- Testing mb_ereg_replace() function with more than expected no. of arguments --\n";
$pattern = b'[a-k]';
$replacement = b'1';
$string = b'string_val';
$option = '';
$extra_arg = 10;
try { var_dump( mb_ereg_replace($pattern, $replacement, $string, $option, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing mb_ereg_replace with one less than the expected number of arguments
echo "\n-- Testing mb_ereg_replace() function with less than expected no. of arguments --\n";
$pattern = b'string_val';
$replacement = b'string_val';
try { var_dump( mb_ereg_replace($pattern, $replacement) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
