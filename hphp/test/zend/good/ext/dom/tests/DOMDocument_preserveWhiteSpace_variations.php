<?php

echo "Load document with preserveWhiteSpace on\n";
$doc = new DOMDocument;
$doc->load(dirname(__FILE__)."/book.xml");
echo $doc->saveXML();


echo "\nLoad document with preserveWhiteSpace off\n";
$doc = new DOMDocument;
$doc->preserveWhiteSpace = false;
$doc->load(dirname(__FILE__)."/book.xml");
echo $doc->saveXML();

?>
