<?php

function foo() {
 return "asd";
 }
function bar() {
 return "bar";
 }

class Bar {
  public function asd() {
 return $this;
 }
}

class Baz {
  public function k() {
 return 12;
 }
}

function main() {
  $k = new Bar;
  $y = new Baz;
  foo();
  $k->asd();
  $y->k();
}

function const_fold() {
  echo foo().bar()."\n";
}

main();
const_fold();
