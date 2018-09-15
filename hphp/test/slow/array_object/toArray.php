<?php


<<__EntryPoint>>
function main_to_array() {
$a = new ArrayObject();
$a['a'] = 'a';
var_dump($a);
var_dump((array) $a);
}
