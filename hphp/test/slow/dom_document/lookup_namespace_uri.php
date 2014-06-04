<?php
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

echo 'Empty: '; var_dump($childElts->item(0)->lookupNamespaceURI(''));
echo 'string: '; var_dump($childElts->item(0)->lookupNamespaceURI('string'));
echo 'null: '; var_dump($childElts->item(0)->lookupNamespaceURI(null));
echo 'array: '; var_dump($childElts->item(0)->lookupNamespaceURI(array()));
