<?php


<<__EntryPoint>>
function main_sort() {
$fruits = array("lemon", "orange", "banana", "apple");
sort(&$fruits);
var_dump($fruits);

$fruits = array("lemon", "orange", "banana", "apple");
rsort(&$fruits);
var_dump($fruits);
}
