<?hh
/* Prototype  : proto string utf8_decode(string data)
 * Description: Converts a UTF-8 encoded string to ISO-8859-1 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing utf8_decode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing utf8_decode() function with Zero arguments --\n";
try { var_dump( utf8_decode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test utf8_decode with one more than the expected number of arguments
echo "\n-- Testing utf8_decode() function with more than expected no. of arguments --\n";
$data = 'string_val';
$extra_arg = 10;
try { var_dump( utf8_decode($data, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
