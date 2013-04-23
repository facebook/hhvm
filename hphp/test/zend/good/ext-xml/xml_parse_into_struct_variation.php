<?php
/* Prototype  : proto int xml_parse_into_struct(resource parser, string data, array &struct, array &index)
 * Description: Parsing a XML document 
 * Source code: ext/xml/xml.c
 * Alias to functions: 
 */

echo "*** Testing xml_parse_into_struct() : variation ***\n";

$simple = "<main><para><note>simple note</note></para><para><note>simple note</note></para></main>";
$p = xml_parser_create();
xml_parse_into_struct($p, $simple, $vals, $index);
xml_parser_free($p);
echo "Index array\n";
print_r($index);
echo "\nVals array\n";
print_r($vals);


echo "Done";
?>