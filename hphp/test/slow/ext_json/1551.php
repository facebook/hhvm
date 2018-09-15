<?php


<<__EntryPoint>>
function main_1551() {
$a = array();
$a[] = &$a;
var_dump($a);
var_dump(json_encode($a));
}
