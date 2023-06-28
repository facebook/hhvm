<?hh


<<__EntryPoint>>
function main_dom_isset() :mixed{
$doc = new DOMDocument();
$doc->loadXML('<x:x xmlns:x="urn:x"/>');
$node = $doc->documentElement;

var_dump(isset($doc->localName));
var_dump(isset($node->localName));
var_dump(isset($node->invalidProperty));
}
