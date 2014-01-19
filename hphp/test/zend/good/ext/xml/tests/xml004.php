<?php
chdir(dirname(__FILE__));

$xp = xml_parser_create();
xml_parser_set_option($xp, XML_OPTION_CASE_FOLDING, false);
xml_set_element_handler($xp, "start_element", "end_element");
$fp = fopen("xmltest.xml", "r");
while ($data = fread($fp, 4096)) {
	xml_parse($xp, $data, feof($fp));
}
xml_parser_free($xp);
$xp = xml_parser_create();
xml_parser_set_option($xp, XML_OPTION_CASE_FOLDING, true);
xml_set_element_handler($xp, "start_element", "end_element");
$fp = fopen("xmltest.xml", "r");
while ($data = fread($fp, 4096)) {
	xml_parse($xp, $data, feof($fp));
}
xml_parser_free($xp);

function start_element($xp, $elem, $attribs)
{
	print "<$elem";
	if (sizeof($attribs)) {
		while (list($k, $v) = each($attribs)) {
			print " $k=\"$v\"";
		}
	}
	print ">\n";
}

function end_element($xp, $elem)
{
	print "</$elem>\n";
}
?>