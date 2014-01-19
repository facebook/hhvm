<?php
function startHandler($parser,$tag,$attr)
{
	var_dump($tag,$attr);
}

function endHandler($parser,$tag)
{ 
	var_dump($tag); 
}

$xmldata = '<?xml version="1.0" encoding="ISO-8859-1"?><äöü üäß="Üäß">ÄÖÜ</äöü>';
$parser = xml_parser_create('ISO-8859-1');
xml_set_element_handler($parser, "startHandler", "endHandler");
xml_parse_into_struct($parser, $xmldata, $struct, $index);
var_dump($struct);
?>