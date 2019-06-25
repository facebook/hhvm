<?hh
/* Prototype  : string mb_substr(string $str, int $start [, int $length [, string $encoding]])
 * Description: Returns part of a string 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass incorrect number of arguments to mb_substr() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_substr() : error conditions ***\n";

//Test mb_substr with one more than the expected number of arguments
echo "\n-- Testing mb_substr() function with more than expected no. of arguments --\n";
$str = 'string_val';
$start = 10;
$length = 10;
$encoding = 'string_val';
$extra_arg = 10;
try { var_dump( mb_substr($str, $start, $length, $encoding, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing mb_substr with one less than the expected number of arguments
echo "\n-- Testing mb_substr() function with less than expected no. of arguments --\n";
$str = 'string_val';
try { var_dump( mb_substr($str) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
