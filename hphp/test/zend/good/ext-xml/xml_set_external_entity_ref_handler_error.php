<?php
/* Prototype  : proto int xml_set_external_entity_ref_handler(resource parser, string hdl)
 * Description: Set up external entity reference handler 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing xml_set_external_entity_ref_handler() : error conditions ***\n";


//Test xml_set_external_entity_ref_handler with one more than the expected number of arguments
echo "\n-- Testing xml_set_external_entity_ref_handler() function with more than expected no. of arguments --\n";

$hdl = 'string_val';
$extra_arg = 10;
var_dump( xml_set_external_entity_ref_handler(null, $hdl, $extra_arg) );

// Testing xml_set_external_entity_ref_handler with one less than the expected number of arguments
echo "\n-- Testing xml_set_external_entity_ref_handler() function with less than expected no. of arguments --\n";

var_dump( xml_set_external_entity_ref_handler(null) );

echo "Done";
?>