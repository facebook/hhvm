<?php


<<__EntryPoint>>
function main_1064() {
$a = 1;
 $c = $b = &$a;
 $b = 2;
 var_dump($a);
 var_dump($c);
}
