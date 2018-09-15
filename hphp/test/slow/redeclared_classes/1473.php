<?php

class A extends Exception {
 public $a = 1;
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

<<__EntryPoint>>
function main_1473() {
if (0) {
  class A {
 public $a = 2;
 }
}
 test();
}
