<?php


<<__EntryPoint>>
function main_1070() {
$a = 10;
 $b = array('test' => &$a);
 var_dump($b);
 $a = 20;
 var_dump($b);
}
