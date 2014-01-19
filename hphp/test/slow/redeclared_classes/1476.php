<?php

class PEAR {
  static function f() {
 PEAR::g();
 }
  function g() {
 echo 'This is g()';
 }
}
if ($x) {
  class PEAR {
}
}
class D1 extends PEAR {
  public $foo;
  private $bar;
  function bar() {
 return $this->foo + $this->bar;
 }
}
class D2 extends D1 {
  public $foo;
  private $bar;
  function bar() {
 return $this->foo + $this->bar;
 }
}
PEAR::f();
