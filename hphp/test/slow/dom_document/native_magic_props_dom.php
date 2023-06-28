<?hh

<<__EntryPoint>>
function main_native_magic_props_dom() :mixed{
$doc = new DOMDocument();
var_dump($doc->version); // native

// Custom Element

$node = $doc->appendChild($doc->createElement('Foo', 'Bar'));
var_dump($node->nodeValue);

// Attr

$node->setAttribute('x', 'y');
var_dump($node->getAttributeNode('x')->nodeName); // inherited
var_dump($node->getAttributeNode('x')->name); // own

// CharacterData, Comment

$doc->loadHTML('<!--comment--><h1 id="x">foo</h1>');
var_dump($doc->firstChild === $doc->doctype); // true
var_dump($doc->doctype->name);

var_dump($doc->doctype->entities->length);

var_dump($doc->firstChild->nextSibling->data); // "comment"

// NodeList, NamedNodeMap

$h1 = $doc->getElementsByTagName('h1');
var_dump($h1->length);
var_dump($h1->item(0)->attributes->length);

// XPath

$xpath = new DOMXPath($doc);
var_dump($xpath->document->doctype->name);
$elements = $xpath->query("//*[@id]");
var_dump($elements->length);
}
