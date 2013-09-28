<?php

function f() {
 throw new Exception('foo');
 }
class X {
  function foo() {
    try {
      f();
    }
 catch (Exception $this) {
      return $this;
    }
  }
}
$x = new X;
$ex = $x->foo();
var_dump($ex->getMessage());
