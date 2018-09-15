<?php

function dump($elems) {
 foreach ($elems as $elem) {
  var_dump($elem->nodeName);
  dump($elem->childNodes);
}
}


<<__EntryPoint>>
function main_1673() {
$xml = <<<HERE
<?xml version="1.0" encoding="ISO-8859-1" ?>
<foo xmlns="http://www.example.com/ns/foo"
     xmlns:fubar="http://www.example.com/ns/fubar">
  <bar><test1 /></bar>
  <bar><test2 /></bar>
  <fubar:bar><test3 /></fubar:bar>
  <fubar:bar><test4 /></fubar:bar>
</foo>
HERE;

$dom = new DOMDocument();
$dom->loadXML($xml);
$doc = $dom->documentElement;
dump($dom->getElementsByTagName('bar'));
dump($doc->getElementsByTagName('bar'));
dump($dom->getElementsByTagNameNS('http://www.example.com/ns/fubar', 'bar'));
dump($doc->getElementsByTagNameNS('http://www.example.com/ns/fubar', 'bar'));
}
