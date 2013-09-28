<?php
/* Prototype  : proto int xml_get_error_code(resource parser)
 * Description: Get XML parser error code 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing xml_get_error_code() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing xml_get_error_code() function with Zero arguments --\n";
var_dump( xml_get_error_code() );

//Test xml_get_error_code with one more than the expected number of arguments
echo "\n-- Testing xml_get_error_code() function with more than expected no. of arguments --\n";

$extra_arg = 10;
var_dump( xml_get_error_code(null, $extra_arg) );

echo "Done";
?>