<?php


<<__EntryPoint>>
function main_505() {
$a = array(1, 'hello', 3.5);
$b = $a;
$b[4] = 'world';
var_dump($a);
var_dump($b);
}
