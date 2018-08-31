<?php


<<__EntryPoint>>
function main_1055() {
$a = array();
$b = new stdClass();
$b->foo = 2;
var_dump($a < $b);
var_dump($a <= $b);
var_dump($a > $b);
var_dump($a >= $b);
var_dump($b < $a);
var_dump($b <= $a);
var_dump($b > $a);
var_dump($b >= $a);
}
