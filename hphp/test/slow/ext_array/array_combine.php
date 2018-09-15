<?php


<<__EntryPoint>>
function main_array_combine() {
$a = array("green", "red", "yellow");
$b = array("avocado", "apple", "banana");
$c = array_combine($a, $b);
var_dump($c);
}
