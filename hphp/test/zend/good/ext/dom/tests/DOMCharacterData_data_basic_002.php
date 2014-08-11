<?php

$document = new DOMDocument;
$root = $document->createElement('root');
$document->appendChild($root);

$cdata = $document->createCDATASection('t');
$root->appendChild($cdata);
print $document->saveXML()."\n";

$cdata->data = 100;
print $document->saveXML()."\n";

?>
