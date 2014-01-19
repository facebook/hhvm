<?php

class A {
 var $v = 10;
 function f() {
 $this->v = 100;
 }
 }
$a = array(array(1, 2, 3), new A());
apc_store('0', $a);
$b = apc_fetch(0);
var_dump($b[1]->v);
$b[1]->f();
var_dump($b[1]->v);
$b[2] = 1;
var_dump($b[1]->v);
