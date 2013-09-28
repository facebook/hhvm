<?php
/* Prototype  : proto int xml_get_current_column_number(resource parser)
 * Description: Get current column number for an XML parser 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing xml_get_current_column_number() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing xml_get_current_column_number() function with Zero arguments --\n";
var_dump( xml_get_current_column_number() );

//Test xml_get_current_column_number with one more than the expected number of arguments
echo "\n-- Testing xml_get_current_column_number() function with more than expected no. of arguments --\n";

$extra_arg = 10;
var_dump( xml_get_current_column_number(null, $extra_arg) );

echo "Done";
?>