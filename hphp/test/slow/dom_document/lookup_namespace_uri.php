<?hh

<<__EntryPoint>>
function main_lookup_namespace_uri() :mixed{
$xml =
 '<?xml version="1.0" encoding="UTF-8"?>
  <rootElement xmlns="http://www.website.org/somens">
    <childElement id="child">
      Some data
    </childElement>
  </rootElement>';

$dom = new DOMDocument('1.0', 'UTF-8');
$dom->loadXML($xml);
$childElts = $dom->getElementsByTagName('childElement');

echo 'Empty: '; var_dump($childElts->item(0)->lookupNamespaceUri(''));
echo 'string: '; var_dump($childElts->item(0)->lookupNamespaceUri('string'));
echo 'null: '; var_dump($childElts->item(0)->lookupNamespaceUri(null));
echo 'array: '; var_dump($childElts->item(0)->lookupNamespaceUri(vec[]));
}
