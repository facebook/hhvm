<?php


<<__EntryPoint>>
function main_1635() {
$node = new SimpleXMLElement('<foo><bar name="value">whoops</bar></foo>');
var_dump((array)$node->bar);
}
