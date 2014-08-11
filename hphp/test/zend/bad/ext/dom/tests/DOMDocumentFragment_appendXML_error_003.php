<?php
$document = new DOMDocument;
$root = $document->createElement('root');
$document->appendChild($root);

$fragment = $document->createDocumentFragment();
@$fragment->appendXML('<foo>is<bar>great</foo>');
$root->appendChild($fragment);
?>
