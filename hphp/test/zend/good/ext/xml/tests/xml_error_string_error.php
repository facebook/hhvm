<?hh
/* Prototype  : proto string xml_error_string(int code)
 * Description: Get XML parser error string 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing xml_error_string() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing xml_error_string() function with Zero arguments --\n";
try { var_dump( xml_error_string() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test xml_error_string with one more than the expected number of arguments
echo "\n-- Testing xml_error_string() function with more than expected no. of arguments --\n";
$code = 10;
$extra_arg = 10;
try { var_dump( xml_error_string($code, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
