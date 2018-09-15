<?php


<<__EntryPoint>>
function main_1119() {
$a = array(0, 1);
$b = array(0, 1);
$a[0] =& $b;
$c =& $a;
unset($a[0][0]);
var_dump($a);
}
