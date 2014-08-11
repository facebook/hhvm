<?php
$dom = new DOMDocument('1.0', 'UTF-8');
$element = $dom->appendChild(new DOMElement('root'));
$comment = new DOMComment("This is the first comment.");
$comment->__construct("This is the second comment.");
$comment = $element->appendChild($comment);
print $dom->saveXML();
?>
