<?hh <<__EntryPoint>> function main(): void {
$dom = new DOMDocument();

$doc = $dom->load(dirname(__FILE__) . "/book.xml", LIBXML_NOBLANKS);
invariant($doc === true, "");

$parent_node = $dom->getElementsByTagName("book")->item(0);
invariant(!is_null($parent_node), "");

$new_node = $dom->createElement('newnode');
invariant($new_node !== false, "");

// getting a sibling as reference node to insert

$ref_node = $dom->getElementsByTagName("book")->item(1);

try {
    $parent_node->insertBefore($new_node, $ref_node);
} catch(DOMException $e) {
    echo $e->getMessage();
}
}
