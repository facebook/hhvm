<?hh
/* Prototype  : proto int xml_parse(resource parser, string data [, int isFinal])
 * Description: Start parsing an XML document 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing xml_parse() : error conditions ***\n";


//Test xml_parse with one more than the expected number of arguments
echo "\n-- Testing xml_parse() function with more than expected no. of arguments --\n";

$data = 'string_val';
$isFinal = false;
$extra_arg = 10;
try { var_dump( xml_parse(null, $data, $isFinal, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing xml_parse with one less than the expected number of arguments
echo "\n-- Testing xml_parse() function with less than expected no. of arguments --\n";

try { var_dump( xml_parse(null) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
