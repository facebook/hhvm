<?php


<<__EntryPoint>>
function main_1067() {
$a = 1;
 $b = &$a;
 $c = 2;
 $d = &$c;
 $b = &$d;
 var_dump($a);
 var_dump($b);
 var_dump($c);
 var_dump($d);
}
