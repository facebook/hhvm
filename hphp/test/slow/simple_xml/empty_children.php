<?php


<<__EntryPoint>>
function main_empty_children() {
$x = new SimpleXMLElement("<foo><x /></foo>");
var_dump($x->x->children()[0]);
}
