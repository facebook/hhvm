<?php
$xml = <<<HERE
<?xml version="1.0" ?>
<foo xmlns="http://www.example.com/ns/foo" />
HERE;

$doc = new DOMDocument();
$doc->loadXML($xml);

$root = $doc->documentElement;

print $root->lookupNamespaceURI(NULL);


?>
