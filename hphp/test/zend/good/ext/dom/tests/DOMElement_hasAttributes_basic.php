<?php
require_once("dom_test.inc");

$dom = new DOMDocument;
$dom->loadXML($xmlstr);
if(!$dom) {
  echo "Error while parsing the document\n";
  exit;
}

$element = $dom->documentElement;

echo "Verify that we have a DOMElement object:\n";
echo get_class($element), "\n";

echo "\nElement should have attributes:\n";
var_dump($element->hasAttributes()); 

$nodelist=$dom->getElementsByTagName('tbody') ; 
$element = $nodelist->item(0);

echo "\nVerify that we have a DOMElement object:\n";
echo get_class($element), "\n";

echo "\nElement should have no attributes:\n"; 
var_dump($element->hasAttributes());


?>
