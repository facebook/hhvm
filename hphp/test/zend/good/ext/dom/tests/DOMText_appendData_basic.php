<?php

$document = new DOMDocument;
$root = $document->createElement('root');
$document->appendChild($root);

$text = $document->createElement('text');
$root->appendChild($text);

$textnode = $document->createTextNode('');
$text->appendChild($textnode);
$textnode->appendData('data');
echo "Text Length (one append): " . $textnode->length . "\n";

$textnode->appendData('><&"');
echo "Text Length (two appends): " . $textnode->length . "\n";

echo "Text Content: " . $textnode->data . "\n";

echo "\n" . $document->saveXML();

?>
===DONE===
