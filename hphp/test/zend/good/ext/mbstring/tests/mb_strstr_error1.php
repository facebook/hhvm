<?php
/* Prototype  : string mb_strstr(string haystack, string needle[, bool part[, string encoding]])
 * Description: Finds first occurrence of a string within another 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_strstr() : error conditions ***\n";


//Test mb_strstr with one more than the expected number of arguments
echo "\n-- Testing mb_strstr() function with more than expected no. of arguments --\n";
$haystack = b'string_val';
$needle = b'string_val';
$part = true;
$encoding = 'string_val';
$extra_arg = 10;
try { var_dump( mb_strstr($haystack, $needle, $part, $encoding, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing mb_strstr with one less than the expected number of arguments
echo "\n-- Testing mb_strstr() function with less than expected no. of arguments --\n";
$haystack = b'string_val';
try { var_dump( mb_strstr($haystack) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

?>
===DONE===
