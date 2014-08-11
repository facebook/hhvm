<?php

$dom = new DOMDocument();

$dom->loadXML('<root/>');

echo $dom->saveXML();

echo "Document has child nodes\n";
var_dump($dom->documentElement->hasChildNodes());

echo "Document has child nodes\n";
$dom->loadXML('<root><a/></root>');
var_dump($dom->documentElement->hasChildNodes());

echo "Remove node and save\n";
$dom->documentElement->removeChild($dom->documentElement->firstChild);
echo $dom->saveXML();

echo "Document has child nodes\n";
var_dump($dom->documentElement->hasChildNodes());

echo "Document with 2 child nodes\n";
$dom->loadXML('<root><a/><b/></root>');
var_dump($dom->documentElement->hasChildNodes());

?>
