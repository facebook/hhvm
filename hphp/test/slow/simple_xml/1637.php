<?php


<<__EntryPoint>>
function main_1637() {
$node = simplexml_load_string('<foo><bar>whoops</bar></foo>');
var_dump((string)$node);
}
