<?hh
/* Prototype  : proto int xml_get_error_code(resource parser)
 * Description: Get XML parser error code 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing xml_get_error_code() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing xml_get_error_code() function with Zero arguments --\n";
try { var_dump( xml_get_error_code() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test xml_get_error_code with one more than the expected number of arguments
echo "\n-- Testing xml_get_error_code() function with more than expected no. of arguments --\n";

$extra_arg = 10;
try { var_dump( xml_get_error_code(null, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
