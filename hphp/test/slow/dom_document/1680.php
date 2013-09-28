<?php

function foo() {
  $html = '<b>Hello</b><i>World</i>';
  $doc = new DOMDocument();
  $element = $doc->createDocumentFragment();
  $element->appendXML($html);
  foreach ($element->childNodes->getIterator() as $child) {
    $element = null;
    $doc = null;
    var_dump($child->nodeValue);
  }
}
foo();
