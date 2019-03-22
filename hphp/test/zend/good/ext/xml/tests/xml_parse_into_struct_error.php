<?php
/* Prototype  : proto int xml_parse_into_struct(resource parser, string data, array &struct, array &index)
 * Description: Parsing a XML document
 * Source code: ext/xml/xml.c
 * Alias to functions:
 */

echo "*** Testing xml_parse_into_struct() : error conditions ***\n";

//Test xml_parse_into_struct with one more than the expected number of arguments
echo "\n-- Testing xml_parse_into_struct() function with more than expected no. of arguments --\n";

$data = 'string_val';
$struct = array(1, 2);
$index = array(1, 2);
$extra_arg = 10;
try { var_dump( xml_parse_into_struct(null, $data, &$struct, &$index, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing xml_parse_into_struct with one less than the expected number of arguments
echo "\n-- Testing xml_parse_into_struct() function with less than expected no. of arguments --\n";

$data = 'string_val';
try { var_dump( xml_parse_into_struct(null, $data) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
