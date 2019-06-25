<?hh
/* Prototype  : proto int xml_get_current_column_number(resource parser)
 * Description: Get current column number for an XML parser 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing xml_get_current_column_number() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing xml_get_current_column_number() function with Zero arguments --\n";
try { var_dump( xml_get_current_column_number() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test xml_get_current_column_number with one more than the expected number of arguments
echo "\n-- Testing xml_get_current_column_number() function with more than expected no. of arguments --\n";

$extra_arg = 10;
try { var_dump( xml_get_current_column_number(null, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
