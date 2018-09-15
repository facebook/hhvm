<?php


<<__EntryPoint>>
function main_1087() {
$a1 = array(&$a1, 1);
 $a2 = $a1;
 unset($a1);
$a2[0][] = 2;
var_dump($a2[0][0][0][2]);
}
