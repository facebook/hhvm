<?hh
/* Prototype  : string mb_regex_encoding([string $encoding])
 * Description: Returns the current encoding for regex as a string. 
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * Test mb_regex_encoding with one more than expected number of arguments
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_regex_encoding() : error conditions ***\n";


echo "\n-- Testing mb_regex_encoding() function with more than expected no. of arguments --\n";
$encoding = 'string_val';
$extra_arg = 10;
try { var_dump( mb_regex_encoding($encoding, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
