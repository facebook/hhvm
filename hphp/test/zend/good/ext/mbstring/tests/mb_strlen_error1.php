<?hh
/* Prototype  : int mb_strlen(string $str [, string $encoding])
 * Description: Get character numbers of a string 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass mb_strlen an incorrect number of arguments to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing mb_strlen() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing mb_strlen() function with Zero arguments --\n";
try { var_dump( mb_strlen() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test mb_strlen with one more than the expected number of arguments
echo "\n-- Testing mb_strlen() function with more than expected no. of arguments --\n";
$str = 'string_val';
$encoding = 'string_val';
$extra_arg = 10;
try { var_dump( mb_strlen($str, $encoding, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
