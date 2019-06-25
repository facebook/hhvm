<?hh
/* Prototype  : proto string utf8_encode(string data)
 * Description: Encodes an ISO-8859-1 string to UTF-8 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing utf8_encode() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing utf8_encode() function with Zero arguments --\n";
try { var_dump( utf8_encode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test utf8_encode with one more than the expected number of arguments
echo "\n-- Testing utf8_encode() function with more than expected no. of arguments --\n";
$data = 'string_val';
$extra_arg = 10;
try { var_dump( utf8_encode($data, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
