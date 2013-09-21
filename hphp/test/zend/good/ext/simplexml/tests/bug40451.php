<?php

$string = <<<XML
<?xml version="1.0"?>
	<Host enable="true">
	 <Name>host.server.com</Name>
	 </Host>
XML;

$xml = simplexml_load_string($string);

$add = $xml->addChild('Host');
$add->Host->addAttribute('enable', 'true');

?>
===DONE===