<?php

class A extends Exception {
 public $a = 1;
 }
if (0) {
  class A {
 public $a = 2;
 }
}
function test() {
try {
  throw new A;
}
 catch (A $e) {
  echo $e->a, '
';
}
}
 test();
