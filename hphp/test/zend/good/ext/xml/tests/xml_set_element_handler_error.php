<?hh
/* Prototype  : proto int xml_set_element_handler(resource parser, string shdl, string ehdl)
 * Description: Set up start and end element handlers 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing xml_set_element_handler() : error conditions ***\n";


//Test xml_set_element_handler with one more than the expected number of arguments
echo "\n-- Testing xml_set_element_handler() function with more than expected no. of arguments --\n";

$hdl = 'string_val';
$extra_arg = 10;
try { var_dump( xml_set_element_handler(null, $hdl, $hdl, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing xml_set_element_handler with one less than the expected number of arguments
echo "\n-- Testing xml_set_element_handler() function with less than expected no. of arguments --\n";

try { var_dump( xml_set_element_handler(null) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
