<?hh <<__EntryPoint>> function main(): void {
$doc = new DOMDocument();
$xpath = new DOMXPath($doc);
$nodes = $xpath->query('*');
echo get_class($nodes), "\n";
var_dump($nodes->length);
$length = $nodes->length;
var_dump(!((bool)$nodes->length ?? false), !((bool)$length ?? false));

$doc->loadXML("<element></element>");
var_dump($doc->firstChild->nodeValue, !((bool)$doc->firstChild->nodeValue ?? false), isset($doc->firstChild->nodeValue));
var_dump(!((bool)$doc->nodeType ?? false), !((bool)$doc->firstChild->nodeType ?? false));
echo "===DONE===\n";
}
