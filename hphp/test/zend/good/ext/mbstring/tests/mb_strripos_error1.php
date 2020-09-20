<?hh
/* Prototype  : int mb_strripos(string haystack, string needle [, int offset [, string encoding]])
 * Description: Finds position of last occurrence of a string within another, case insensitive 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

/*
 * Test how mb_strripos behaves when passed an incorrect number of arguments
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strripos() : error conditions ***\n";


//Test mb_strripos with one more than the expected number of arguments
echo "\n-- Testing mb_strripos() function with more than expected no. of arguments --\n";
$haystack = b'string_val';
$needle = b'string_val';
$offset = 10;
$encoding = 'string_val';
$extra_arg = 10;
try { var_dump( mb_strripos($haystack, $needle, $offset, $encoding, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing mb_strripos with one less than the expected number of arguments
echo "\n-- Testing mb_strripos() function with less than expected no. of arguments --\n";
$haystack = b'string_val';
try { var_dump( mb_strripos($haystack) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
