<?php

$doc = simplexml_load_string('<?xml version="1.0"?><root xmlns:foo="http://example.com"><foo:b1>c1</foo:b1><foo:b2>c2</foo:b2><foo:b2>c3</foo:b2></root>');
 $foo_ns_bar = $doc->children('http://example.com');
var_dump($doc->getName());
foreach ($foo_ns_bar as $v) var_dump((string)$v);
var_dump($foo_ns_bar->getName());
var_dump(count($foo_ns_bar->b1));
var_dump((string)$foo_ns_bar->b1);
var_dump((string)$foo_ns_bar->b1[0]);
foreach ($foo_ns_bar->b1 as $v) var_dump((string)$v);
var_dump(count($foo_ns_bar->b2));
var_dump((string)$foo_ns_bar->b2[0]);
var_dump((string)$foo_ns_bar->b2[1]);
foreach ($foo_ns_bar->b2 as $v) var_dump((string)$v);
