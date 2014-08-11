<?php

$document = new DOMDocument;
$root = $document->createElement('root');
$document->appendChild($root);
$attr = $root->setAttribute('category', 'books');
$document->removeChild($root);
$root = null;
var_dump($attr->ownerElement);
?>
