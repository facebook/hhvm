<?php


<<__EntryPoint>>
function main_1058() {
$a = array(1, 'test');
 $b = $a;
 $c = &$b[0];
 $c = 10;
 var_dump($a, $b);
}
