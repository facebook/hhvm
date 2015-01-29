<?php

class MyDocument extends DOMDocument {
  public function __get($name) {
    return "MyDocument::__get: $name";
  }
}

class MyElement extends DOMElement {
  public function __get($name) {
    return "MyElement::__get: $name";
  }
}

// Custom doc

$doc = new MyDocument();
var_dump($doc->version); // native
var_dump($doc->nonExisting); // user

// Custom Element

$doc->registerNodeClass('DOMElement', 'MyElement');
$node = $doc->appendChild($doc->createElement('Foo', 'Bar'));
var_dump($node->nodeValue);
var_dump($node->nonExisting);

// Attr

$node->setAttribute('x', 'y');
var_dump($node->getAttributeNode('x')->nodeName); // inherited
var_dump($node->getAttributeNode('x')->name); // own

// CharacterData, Comment

$doc->loadHTML('<!--comment--><h1 id="x">foo</h1>');
var_dump($doc->firstChild === $doc->doctype); // true
var_dump($doc->doctype->name);

var_dump($doc->doctype->entities->length);

var_dump($doc->firstChild->nextSibling->data); // "comment"

// NodeList, NamedNodeMap

$h1 = $doc->getElementsByTagName('h1');
var_dump($h1->length);
var_dump($h1->item(0)->attributes->length);

// XPath

$xpath = new DOMXpath($doc);
var_dump($xpath->document->doctype->name);
$elements = $xpath->query("//*[@id]");
var_dump($elements->length);
