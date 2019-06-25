<?hh
/* Prototype  : proto int xml_parser_get_option(resource parser, int option)
 * Description: Get options from an XML parser 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing xml_parser_get_option() : error conditions ***\n";


//Test xml_parser_get_option with one more than the expected number of arguments
echo "\n-- Testing xml_parser_get_option() function with more than expected no. of arguments --\n";

$option = 10;
$extra_arg = 10;
try { var_dump( xml_parser_get_option(null, $option, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing xml_parser_get_option with one less than the expected number of arguments
echo "\n-- Testing xml_parser_get_option() function with less than expected no. of arguments --\n";

try { var_dump( xml_parser_get_option(null) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
