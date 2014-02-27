<?php
$xml = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><root />');
$n = $xml->addChild("node", "value");
$n->addAttribute("a", "b");
$n->addAttribute("c", "d", "http://bar.com");
$n->addAttribute("foo:e", "f", "http://bar.com");
print_r($xml->asXml());
?>
===DONE===