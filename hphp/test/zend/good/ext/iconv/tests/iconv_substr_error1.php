<?hh
/* Prototype  : string iconv_substr(string str, int offset, [int length, string charset])
 * Description: Returns part of a string 
 * Source code: ext/iconv/iconv.c
 */

/*
 * Pass incorrect number of arguments to iconv_substr() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing iconv_substr() : error conditions ***\n";

//Test iconv_substr with one more than the expected number of arguments
echo "\n-- Testing iconv_substr() function with more than expected no. of arguments --\n";
$str = 'string_val';
$start = 10;
$length = 10;
$encoding = 'string_val';
$extra_arg = 10;
try { var_dump( iconv_substr($str, $start, $length, $encoding, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing iconv_substr with one less than the expected number of arguments
echo "\n-- Testing iconv_substr() function with less than expected no. of arguments --\n";
$str = 'string_val';
try { var_dump( iconv_substr($str) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
