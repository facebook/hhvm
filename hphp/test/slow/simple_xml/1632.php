<?php


<<__EntryPoint>>
function main_1632() {
$x = new SimpleXMLElement('<foo><bar>0</bar></foo>');
var_dump((bool)$x->bar);
}
