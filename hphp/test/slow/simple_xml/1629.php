<?php

function addChildNode(SimpleXMLElement $parent, SimpleXMLElement $node) {
  $newchild = $parent->addChild($node->getName(), (string)$node);
  foreach ($node->attributes() as $name => $value) {
    $newchild->addAttribute($name, $value);
  }
  foreach ($node->children() as $child) {
    addChildNode($newchild, $child);
  }
}

$xmlreq = '<a><item><node><sub>1st</sub><sub>2nd</sub></node></item></a>';
$quote = simplexml_load_string($xmlreq);
$req = new SimpleXMLElement('<node/>');
foreach ($quote->attributes() as $name => $value) {
  $req->addAttribute($name, $value);
}
foreach ($quote->children() as $child) {
  addChildNode($req, $child);
}

$vertex = new SimpleXMLElement('<root/>');
addChildNode($vertex, $req);
var_dump($vertex->asXML());
