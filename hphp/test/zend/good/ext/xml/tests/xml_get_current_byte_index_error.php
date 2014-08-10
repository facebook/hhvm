<?php
/* Prototype  : proto int xml_get_current_byte_index(resource parser)
 * Description: Get current byte index for an XML parser 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing xml_get_current_byte_index() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing xml_get_current_byte_index() function with Zero arguments --\n";
var_dump( xml_get_current_byte_index() );

//Test xml_get_current_byte_index with one more than the expected number of arguments
echo "\n-- Testing xml_get_current_byte_index() function with more than expected no. of arguments --\n";

$extra_arg = 10;
var_dump( xml_get_current_byte_index(null, $extra_arg) );

echo "Done";
?>
