<?php

function f(&$a, &$b) {
 $a = 1;
 $b = 2;
 return 3;
 }
class A {
 }
function test() {
  $a = array();
 f($a[0], $a[1]);
 var_dump($a);
  $a = array();
 $a[0] = f($a[1], $a[2]);
 var_dump($a);
  $a = new A();
 f($a->f, $a->g);
 var_dump($a);
}
test();
