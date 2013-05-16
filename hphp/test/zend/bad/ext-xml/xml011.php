<?php
function start_elem($parser,$name,$attribs) {
   echo "<$name>";
}
function end_elem()
{
   echo "</$name>";
}

$xml = '<text>start<b /> This &amp; that</text>';

$parser = xml_parser_create();
xml_parse_into_struct($parser, $xml, $vals, $index);
print_r($vals);
xml_parser_free($parser);

echo "\nChange to empty end handler\n";
$parser = xml_parser_create();
xml_parser_set_option($parser,XML_OPTION_CASE_FOLDING,0);
xml_set_element_handler($parser,'start_elem','end_elem');
xml_set_element_handler($parser,'start_elem',NULL);
xml_parse($parser, $xml, TRUE);

xml_parser_free($parser);
echo "\nDone\n";
?>