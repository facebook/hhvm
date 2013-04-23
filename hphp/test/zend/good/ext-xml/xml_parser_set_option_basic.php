<?php
/* Prototype  : proto bool xml_set_notation_decl_handler  ( resource $parser  , callback $handler  )
 * Description: Sets the notation declaration handler function for the XML parser.
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "Simple testcase for xml_parser_get_option() function\n";

$parser = xml_parser_create_ns();

var_dump(xml_parser_get_option($parser, XML_OPTION_CASE_FOLDING));
var_dump(xml_parser_get_option($parser, XML_OPTION_TARGET_ENCODING));

var_dump(xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 1));
var_dump(xml_parser_set_option($parser, XML_OPTION_TARGET_ENCODING, "ISO-8859-1"));

var_dump(xml_parser_get_option($parser, XML_OPTION_CASE_FOLDING));
var_dump(xml_parser_get_option($parser, XML_OPTION_TARGET_ENCODING));

var_dump(xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0));
var_dump(xml_parser_set_option($parser, XML_OPTION_TARGET_ENCODING, "UTF-8"));

var_dump(xml_parser_get_option($parser, XML_OPTION_CASE_FOLDING));
var_dump(xml_parser_get_option($parser, XML_OPTION_TARGET_ENCODING));

var_dump(xml_parser_set_option($parser, XML_OPTION_TARGET_ENCODING, "US-ASCII"));
var_dump(xml_parser_get_option($parser, XML_OPTION_TARGET_ENCODING));

xml_parser_free( $parser );

echo "Done\n";
?>