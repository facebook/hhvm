<?php
/* Prototype  : string mb_decode_mimeheader(string string)
 * Description: Decodes the MIME "encoded-word" in the string 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_decode_mimeheader() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing mb_decode_mimeheader() function with Zero arguments --\n";
try { var_dump( mb_decode_mimeheader() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test mb_decode_mimeheader with one more than the expected number of arguments
echo "\n-- Testing mb_decode_mimeheader() function with more than expected no. of arguments --\n";
$string = 'string_val';
$extra_arg = 10;
try { var_dump( mb_decode_mimeheader($string, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

?>
===DONE===
