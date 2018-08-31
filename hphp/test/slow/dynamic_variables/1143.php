<?php


<<__EntryPoint>>
function main_1143() {
$a = 'test';
 $b = 'a';
 $c = &$$b;
 $c = 10;
 var_dump($a);
}
