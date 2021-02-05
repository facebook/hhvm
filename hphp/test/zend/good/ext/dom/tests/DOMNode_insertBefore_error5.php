<?hh <<__EntryPoint>> function main(): void {
$dom = new DOMDocument();

$doc = $dom->load(dirname(__FILE__) . "/book.xml", LIBXML_NOBLANKS);
invariant($doc === true, "");

$parent_node = $dom->getElementsByTagName("book")->item(0);
invariant(!is_null($parent_node), "");

$new_node = $dom->createElement('newnode');
invariant($new_node !== false, "");

// creating a new node (descendant) and getting it as the refnode

$ref_node = $dom->createElement('newnode3');
$parent_node->childNodes->item(0)->appendChild($ref_node);
$dom->saveXML();

try {
    $parent_node->insertBefore($new_node, $ref_node);
} catch(DOMException $e) {
    echo $e->getMessage();
}
}
