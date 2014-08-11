<?php

$xml = <<< EOXML
<?xml version="1.0" encoding="utf-8"?>
<courses>
	<!-- Hello World! -->
	<aNode>
		<childNode>
			<childlessNode />
		</childNode>
	</aNode>
</courses>
EOXML;

$dom = new DOMDocument();
$dom->loadXML($xml);
$root = $dom->documentElement;
$directory = dirname(__FILE__);

$filename = $directory."/tmp_dom_savexml".time();
var_dump($dom->save($filename));
$result = file_get_contents($filename);
var_dump($result == $dom->saveXML());

unlink($filename);

