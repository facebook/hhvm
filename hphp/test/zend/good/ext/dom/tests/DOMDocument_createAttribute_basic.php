<?php

$doc = new DOMDocument;

$node = $doc->createElement("para");
$newnode = $doc->appendChild($node);

// A pass case.
$test_attribute = $doc->createAttribute("hahaha");
$node->appendChild($test_attribute);

echo $doc->saveXML();

?>
