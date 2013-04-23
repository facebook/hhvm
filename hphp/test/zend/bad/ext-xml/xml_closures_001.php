<?php
chdir(dirname(__FILE__));

$start_element = function ($xp, $elem, $attribs)
{
	print "<$elem";
	if (sizeof($attribs)) {
		while (list($k, $v) = each($attribs)) {
			print " $k=\"$v\"";
		}
	}
	print ">\n";
};

$end_element = function ($xp, $elem)
{
	print "</$elem>\n";
};

$xp = xml_parser_create();
xml_parser_set_option($xp, XML_OPTION_CASE_FOLDING, false);
xml_set_element_handler($xp, $start_element, $end_element);
$fp = fopen("xmltest.xml", "r");
while ($data = fread($fp, 4096)) {
	xml_parse($xp, $data, feof($fp));
}
xml_parser_free($xp);

?>