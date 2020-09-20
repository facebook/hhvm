<?hh
/* Prototype  : int mb_strpos(string $haystack, string $needle [, int $offset [, string $encoding]])
 * Description: Find position of first occurrence of a string within another 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test how mb_strpos behaves when passed an incorrect number of arguments
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strpos() : error conditions ***\n";


//Test mb_strpos with one more than the expected number of arguments
echo "\n-- Testing mb_strpos() function with more than expected no. of arguments --\n";
$haystack = 'string_val';
$needle = 'string_val';
$offset = 10;
$encoding = 'string_val';
$extra_arg = 10;
try { var_dump( mb_strpos($haystack, $needle, $offset, $encoding, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing mb_strpos with one less than the expected number of arguments
echo "\n-- Testing mb_strpos() function with less than expected no. of arguments --\n";
$haystack = 'string_val';
try { var_dump( mb_strpos($haystack) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
