<?php
error_reporting(E_ALL & ~E_NOTICE);
$root = simplexml_load_string('<?xml version="1.0"?>
<root xmlns:reserved="reserved-ns" xmlns:special="special-ns">
 <reserved:child>Hello</reserved:child>
 <special:child>World</special:child>
</root>
');

var_dump($root->children('reserved-ns')->child);
var_dump($root->children('special-ns')->child);
var_dump((string)$root->children('reserved-ns')->child);
var_dump((string)$root->children('special-ns')->child);
var_dump($root->child);
?>
===DONE===