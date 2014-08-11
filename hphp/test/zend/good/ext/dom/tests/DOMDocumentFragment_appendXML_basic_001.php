<?php
$document = new DOMDocument;
$root = $document->createElement('root');
$document->appendChild($root);

$fragment = $document->createDocumentFragment();
$fragment->appendXML('<foo id="baz">bar</foo>');
$root->appendChild($fragment);

print $document->saveXML();
?>
