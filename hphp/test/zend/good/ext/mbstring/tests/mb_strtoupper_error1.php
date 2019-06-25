<?hh
/* Prototype  : string mb_strtoupper(string $sourcestring [, string $encoding]
 * Description: Returns a uppercased version of $sourcestring
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass an incorrect number of arguments to mb_strtoupper() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strtoupper() : error conditions ***\n";

//Test mb_strtoupper with one more than the expected number of arguments
echo "\n-- Testing mb_strtoupper() function with more than expected no. of arguments --\n";
$sourcestring = 'string_value';
$encoding = 'UTF-8';
$extra_arg = 10;
try { var_dump( mb_strtoupper($sourcestring, $encoding, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test mb_strtoupper with zero arguments
echo "\n-- Testing mb_strtoupper() function with zero arguments --\n";
try { var_dump( mb_strtoupper() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
