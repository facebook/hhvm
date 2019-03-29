<?php


<<__EntryPoint>>
function main_42() {
$info = array('coffee', 'brown', 'caffeine');
$a = array();
list($a[0], $a[1], $a[2]) = $info;
var_dump($a);
}
