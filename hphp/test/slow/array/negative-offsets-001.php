<?php


<<__EntryPoint>>
function main_negative_offsets_001() {
$a = array( -1 => 0);
array_pop(&$a);
$a[] = 1;
$a[] = 2;
var_dump($a);
}
