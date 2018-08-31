<?php


<<__EntryPoint>>
function main_1069() {
$a = array();
 $b = 10;
 $a[] = &$b;
 $b = 20;
 var_dump($a);
}
