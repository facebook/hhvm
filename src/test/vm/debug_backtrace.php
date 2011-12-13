<?php

class A {
  static function foo() {
    var_dump(debug_backtrace());
  }

  function bar($a, $b) {
    $this->foo();
  }
}

function bar() {
  $a = new A();
  $a->bar(1, "str", array(1, 2, 3));
}

function foo() {
  call_user_func("bar");
}

foo();
