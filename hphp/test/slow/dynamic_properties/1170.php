<?php

class A {
 }
 function f(&$a) {
 $a = 1000;
 }
 $a = new A();
 $f = 10;
 $a->$f = 100;
 var_dump($a);
 var_dump((array)$a);
 $f = 100;
 f($a->$f);
 foreach ($a as $k => &$v) {
 var_dump($k);
 $v = 1;
 }
 var_dump($a);

