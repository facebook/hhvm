<?php

class A {
  function foo() {
    $msg = "b";
    $reg = "/./e";
    $repl = "\$this->bar()";
    $x = preg_replace($reg, $repl, $msg);
    var_dump($x);
  }

  function bar() {
    return "Hi";
  }

  static function baz() {
    var_dump(get_called_class());
  }
}

error_reporting(E_ALL & ~E_NOTICE & ~E_DEPRECATED);
$a = new A();
$a->foo();
A::baz();
