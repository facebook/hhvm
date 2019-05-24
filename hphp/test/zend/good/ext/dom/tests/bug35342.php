<?php <<__EntryPoint>> function main() {
$dom = new DOMDocument();
$dom->loadXML("<root><foo>foobar</foo><foo>foobar#2</foo></root>");

$nodelist = $dom->getElementsByTagName("foo");

var_dump($nodelist->length, isset($nodelist->length), isset($nodelist->foo));
}
