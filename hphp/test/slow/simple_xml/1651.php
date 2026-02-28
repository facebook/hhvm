<?hh


<<__EntryPoint>>
function main_1651() :mixed{
$doc = new SimpleXMLElement('<?xml version="1.0"?><root><node><option>1</option></node></root>');
 unset($doc->node->option);
 var_dump($doc->asXML());
}
