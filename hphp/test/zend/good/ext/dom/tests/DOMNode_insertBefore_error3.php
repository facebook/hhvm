<?hh <<__EntryPoint>> function main(): void {
$dom = new DOMDocument();

$doc = $dom->load(dirname(__FILE__) . "/book.xml", LIBXML_NOBLANKS);
invariant($doc === true, "doc should be true");

$parent_node = $dom->getElementsByTagName("book")->item(0);
invariant(!is_null($parent_node), "parent_node should not be null");

$new_node = $dom->createElement('newnode');
invariant($new_node !== false, "new_node should not be false");

// getting the parent node as reference node to insert

$ref_node = $dom->getElementsByTagName("book")->item(0)->parentNode;
invariant(!is_null($ref_node), "ref_node should not be null");

try {
    $parent_node->insertBefore($new_node, $ref_node);
} catch(DOMException $e) {
    echo $e->getMessage();
}
}
