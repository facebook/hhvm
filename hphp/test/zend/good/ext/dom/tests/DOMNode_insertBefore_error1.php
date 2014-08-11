<?php

$doc1 = new DOMDocument();
$doc2 = new DOMDocument();

$node_in_doc1 = $doc1->createElement("foo");
$node_in_doc2 = $doc2->createElement("bar");

try {
	$node_in_doc2->insertBefore($node_in_doc1);
} catch(DOMException $e) {
	echo $e->getMessage();
}

?>
