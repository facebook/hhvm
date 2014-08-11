<?php
$dom = new DOMDocument();

$doc = $dom->load(dirname(__FILE__) . "/book.xml", LIBXML_NOBLANKS);
assert('$doc === true');

$parent_node = $dom->getElementsByTagName("book")->item(0);
assert('!is_null($parent_node)');

$new_node = $dom->createElement('newnode');
assert('$new_node !== false');

// getting the parent node as reference node to insert

$ref_node = $dom->getElementsByTagName("book")->item(0)->parentNode;
assert('!is_null($ref_node)');

try {
    $parent_node->insertBefore($new_node, $ref_node);
} catch(DOMException $e) {
	echo $e->getMessage();
}

?>
