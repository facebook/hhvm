<?hh


<<__EntryPoint>>
function main_simplexml_import_dom_element() :mixed{
$doc = new DOMDocument;
$element = $doc->createElement('root');
$xml = simplexml_import_dom($element);
var_dump($xml->asXML());
}
