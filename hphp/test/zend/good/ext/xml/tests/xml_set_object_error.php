<?hh
/* Prototype  : proto int xml_set_object(resource parser, object obj)
 * Description: Set up object which should be used for callbacks
 * Source code: ext/xml/xml.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing xml_set_object() : error conditions ***\n";


//Test xml_set_object with one more than the expected number of arguments
echo "\n-- Testing xml_set_object() function with more than expected no. of arguments --\n";

//WARNING: Unable to initialise parser of type resource

$obj = new stdClass();
$extra_arg = 10;
try { var_dump( xml_set_object(null, $obj, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing xml_set_object with one less than the expected number of arguments
echo "\n-- Testing xml_set_object() function with less than expected no. of arguments --\n";

//WARNING: Unable to initialise parser of type resource

try { var_dump( xml_set_object(null) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
