<?php

class X {
  static public $val = array(1,2,3);
  function foo() {
    list($a, $b) = self::$val;
    var_dump($a, $b);
  }
}
$x = new X;
$x->foo();
X::$val = null;
