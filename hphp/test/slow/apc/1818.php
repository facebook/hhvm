<?php

class A {
 var $i = 10;
 }
$a = array(new A);
apc_store('key1', $a);
$b = apc_fetch('key1');
$c = $b[0];
$c->i = 100;
apc_store('key2', $b);
$t = apc_fetch('key2');
var_dump($t[0]->i);
