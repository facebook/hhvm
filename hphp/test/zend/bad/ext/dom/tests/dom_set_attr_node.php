<?php

$xml = <<<HERE
<?xml version="1.0" ?>
<root a="b" />
HERE;

$xml2 = <<<HERE
<?xml version="1.0" ?>
<doc2 />
HERE;

$dom = new DOMDocument();
$dom->loadXML($xml);
$root = $dom->documentElement;
$attr = $root->getAttributeNode('a');

$dom2 = new DOMDocument();
$dom2->loadXML($xml2);
$root2 = $dom2->documentElement;
try {
	$root2->setAttributeNode($attr);
} catch (domexception $e) {
ob_start();
	var_dump($e);
	$contents = ob_get_contents();
	ob_end_clean();
	echo preg_replace('/object\(DOMAttr\).+\{.*?\}/s', 'DOMAttr', $contents);
} 

?>
