<?php

$document = new DOMDocument;
$root = $document->createElement('root');
$document->appendChild($root);

$cdata = $document->createCDATASection('testfest');
$root->appendChild($cdata);
print $cdata->substringData(1, 6);

?>
