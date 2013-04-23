<?php

$XML = <<<XML
<?xml version="1.0"?>
<ns1:listOfAwards xmlns:ns1="http://www.fpdsng.com/FPDS">
<ns1:count>
<ns1:total>867</ns1:total>
</ns1:count>
</ns1:listOfAwards>
XML;

$xml_parser = xml_parser_create();
xml_parser_set_option($xml_parser, XML_OPTION_SKIP_TAGSTART, 4);
xml_parse_into_struct($xml_parser, $XML, $vals, $index);
echo 'Index array' . PHP_EOL;
print_r($index);
echo 'Vals array' . PHP_EOL;
print_r($vals);
xml_parser_free($xml_parser);

function startElement($parser, $name, $attribs) { echo $name . PHP_EOL; }
function endElement($parser, $name) { echo $name . PHP_EOL; }
$xml_parser = xml_parser_create();
xml_set_element_handler($xml_parser, 'startElement', 'endElement');
xml_parser_set_option($xml_parser, XML_OPTION_SKIP_TAGSTART, 4);
xml_parse($xml_parser, $XML);
xml_parser_free($xml_parser);

?>