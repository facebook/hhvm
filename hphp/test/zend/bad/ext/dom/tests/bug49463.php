<?php

$doc = new DOMDocument('1.0', 'utf-8');
$root = $doc->createElementNS('http://purl.org/rss/1.0/','rdf:RDF');
$doc->appendChild($root);
$root->setAttributeNS("http://www.w3.org/2000/xmlns/","xmlns","http://purl.org/rss/1.0/" );

echo $doc->saveXML()."\n";
?>
