<?php


<<__EntryPoint>>
function main_ns_null_deref() {
$xml = new SimpleXMLElement("X",1);
var_dump($xml->getDocNamespaces());
}
