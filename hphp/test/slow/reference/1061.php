<?php


<<__EntryPoint>>
function main_1061() {
$a = 1;
 $b = &$a;
 $c = $b;
 $a = 2;
 var_dump($b);
 var_dump($c);
}
