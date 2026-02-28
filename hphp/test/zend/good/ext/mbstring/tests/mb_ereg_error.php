<?hh
/* Prototype  : int mb_ereg(string $pattern, string $string [, array $registers])
 * Description: Regular expression match for multibyte string
 * Source code: ext/mbstring/php_mbregex.c
 */

/*
 * Test behaviour of mb_ereg() when passed an incorrcect number of arguments
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_ereg() : error conditions ***\n";


//Test mb_ereg with one more than the expected number of arguments
echo "\n-- Testing mb_ereg() function with more than expected no. of arguments --\n";
$pattern = b'string_val';
$string = b'string_val';
$registers = vec[1, 2];
$extra_arg = 10;
try { var_dump( mb_ereg($pattern, $string, inout $registers, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing mb_ereg with one less than the expected number of arguments
echo "\n-- Testing mb_ereg() function with less than expected no. of arguments --\n";
$pattern = b'string_val';
try { var_dump( mb_ereg($pattern) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
