<?php
chdir(dirname(__FILE__));

class myclass
{
	function startElement($parser, $name, $attribs) 
	{
		print '{'.$name;
		if (sizeof($attribs)) {
			while (list($k, $v) = each($attribs)) {
				print " $k=\"$v\"";
			}
		}
		print '}';
	}
	function endElement($parser, $name)
	{
		print '{/'.$name.'}';
	}
	function characterData($parser, $data)
	{
		print '{CDATA['.$data.']}';
	}
	function PIHandler($parser, $target, $data)
	{
		print '{PI['.$target.','.$data.']}';
	}
	function defaultHandler($parser, $data)
	{
		if (substr($data, 0, 1) == "&" && substr($data, -1, 1) == ";") {
			print '{ENTREF['.$data.']}';
		} else {
			print '{?['.$data.']}';
		}
	}
	function externalEntityRefHandler($parser, $openEntityNames, $base,	$systemId, $publicId)
	{
		print '{EXTENTREF['.$openEntityNames.','.$base.','.$systemId.','.$publicId."]}\n";
		return true;
	}
}

$xml_parser = xml_parser_create();
$obj = new myclass;
xml_set_object($xml_parser, $obj);
xml_parser_set_option($xml_parser, XML_OPTION_CASE_FOLDING, 1);
xml_set_element_handler($xml_parser, "startElement", "endElement");
xml_set_character_data_handler($xml_parser, "characterData");
xml_set_processing_instruction_handler($xml_parser, "PIHandler");
xml_set_default_handler($xml_parser, "defaultHandler");
xml_set_external_entity_ref_handler($xml_parser, "externalEntityRefHandler");

if (!($fp = @fopen("xmltest.xml", "r"))) {
	die("could not open XML input");
}

while ($data = fread($fp, 4096)) {
	if (!xml_parse($xml_parser, $data, feof($fp))) {
		die(sprintf("XML error: %s at line %d\n",
		xml_error_string(xml_get_error_code($xml_parser)),
		xml_get_current_line_number($xml_parser)));
	}
}
print "parse complete\n";
xml_parser_free($xml_parser);
?>