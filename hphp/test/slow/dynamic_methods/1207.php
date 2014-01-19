<?php

class A {
 function test($a, $b) {
 var_dump($a, $b);
}
 }
 $m = 'test';
 $o = new A();
$ar = array(0,1);
 $st = 'abc';
$o->$m($ar[0], $st[0]);
 A::$m($ar[1], $st[1]);
