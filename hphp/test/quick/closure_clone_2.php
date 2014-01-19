<?php

trait T {
  function f() {
    return function ($a) {
      if ($a) {
        return $this->foo;
      }
    };
  }
}

class X {
  private $foo = 1;
  use T;
}

class Y {
  private $foo = 2;
  use T;
}

function test() {
  $x = new X;
  $c = $x->f();
  var_dump($c(true));

  $y = new Y;
  $c = $y->f();
  var_dump($c("foo"));
}

test();
