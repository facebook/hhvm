<?php
$file = dirname(__FILE__).'/book.xml';
$doc = new DOMDocument();
$doc->load($file);
$nodes = $doc->getElementsByTagName('title');
foreach($nodes as $node) {
	var_dump($node->getNodePath());
}
?>
