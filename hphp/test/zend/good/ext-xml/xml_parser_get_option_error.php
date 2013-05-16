<?php
/* Prototype  : proto int xml_parser_get_option(resource parser, int option)
 * Description: Get options from an XML parser 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing xml_parser_get_option() : error conditions ***\n";


//Test xml_parser_get_option with one more than the expected number of arguments
echo "\n-- Testing xml_parser_get_option() function with more than expected no. of arguments --\n";

$option = 10;
$extra_arg = 10;
var_dump( xml_parser_get_option(null, $option, $extra_arg) );

// Testing xml_parser_get_option with one less than the expected number of arguments
echo "\n-- Testing xml_parser_get_option() function with less than expected no. of arguments --\n";

var_dump( xml_parser_get_option(null) );
echo "Done";
?>