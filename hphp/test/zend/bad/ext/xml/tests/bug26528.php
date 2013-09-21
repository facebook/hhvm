<?php
	$sample = "<?xml version=\"1.0\"?><test attr=\"angle&lt;bracket\"/>";
	$parser = xml_parser_create();
	$res = xml_parse_into_struct($parser,$sample,$vals,$index);
	xml_parser_free($parser);
	var_dump($vals);
?>