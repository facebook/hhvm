<?php


<<__EntryPoint>>
function main_add_child() {
$x = new SimpleXMLElement('<foo/>');
$x->addChild('foobar');
var_dump((string)$x->addChild('hello', 'world'));
var_dump((string)$x);
var_dump($x);
foreach ($x->children() as $child) {
  var_dump((string)$child);
}
}
