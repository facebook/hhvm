<?php
<<__EntryPoint>> function main() {
$doc = new DOMDocument;

$doc->load(dirname(__FILE__)."/book.xml");

$result = $doc->schemaValidate('');
var_dump($result);
}
