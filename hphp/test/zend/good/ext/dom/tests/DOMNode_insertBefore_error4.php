<?php
$dom = new DOMDocument();

$doc = $dom->load(dirname(__FILE__) . "/book.xml", LIBXML_NOBLANKS);
assert('$doc === true');

$parent_node = $dom->getElementsByTagName("book")->item(0);
assert('!is_null($parent_node)');

$new_node = $dom->createElement('newnode');
assert('$new_node !== false');

// could be a brand new node

$ref_node = $dom->createElement('newnode2');

try {
    $parent_node->insertBefore($new_node, $ref_node);
} catch(DOMException $e) {
	echo $e->getMessage();
}

?>
