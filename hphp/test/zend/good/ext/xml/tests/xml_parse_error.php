<?php
/* Prototype  : proto int xml_parse(resource parser, string data [, int isFinal])
 * Description: Start parsing an XML document 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing xml_parse() : error conditions ***\n";


//Test xml_parse with one more than the expected number of arguments
echo "\n-- Testing xml_parse() function with more than expected no. of arguments --\n";

$data = 'string_val';
$isFinal = false;
$extra_arg = 10;
var_dump( xml_parse(null, $data, $isFinal, $extra_arg) );

// Testing xml_parse with one less than the expected number of arguments
echo "\n-- Testing xml_parse() function with less than expected no. of arguments --\n";

var_dump( xml_parse(null) );

echo "Done";
?>
