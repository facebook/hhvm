<?php

class A {
  public function foo() {
    unset($this);
    var_dump($this);
  }
  public static function bar() {
    unset($this);
    var_dump($this);
  }
}
function goo() {
  unset($this);
  var_dump($this);
}
$obj = new A;
$obj->foo();

$obj->bar();
A::bar();
goo();
unset($this);
var_dump($this);
