<?php


<<__EntryPoint>>
function main_numeric_dynprops() {
$a = (object)array(0 => '#');
$b = clone $a;
var_dump($b);
}
