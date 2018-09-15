<?php


<<__EntryPoint>>
function main_1068() {
$a = 10;
 $b = array(&$a);
 var_dump($b);
 $a = 20;
 var_dump($b);
}
