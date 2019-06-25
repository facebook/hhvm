<?hh
/* Prototype  : string mb_strtolower(string $sourcestring [, string $encoding])
 * Description: Returns a lowercased version of $sourcestring
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass an incorrect number of arguments to mb_strtolower() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strtolower() : error conditions***\n";

//Test mb_strtolower with one more than the expected number of arguments
echo "\n-- Testing mb_strtolower() function with more than expected no. of arguments --\n";
$sourcestring = 'string_value';
$encoding = 'UTF-8';
$extra_arg = 10;
try { var_dump( mb_strtolower($sourcestring, $encoding, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test mb_strtolower with zero arguments
echo "\n-- Testing mb_strtolower() function with zero arguments --\n";
try { var_dump( mb_strtolower() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
