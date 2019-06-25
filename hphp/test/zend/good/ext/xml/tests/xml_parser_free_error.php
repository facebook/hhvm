<?hh
/* Prototype  : proto int xml_parser_free(resource parser)
 * Description: Free an XML parser 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing xml_parser_free() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing xml_parser_free() function with Zero arguments --\n";
try { var_dump( xml_parser_free() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test xml_parser_free with one more than the expected number of arguments
echo "\n-- Testing xml_parser_free() function with more than expected no. of arguments --\n";

$extra_arg = 10;
try { var_dump( xml_parser_free(null, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
