<?php

class X {
  private $priv = 'X';
  function foo() {
    return function ($x) {
      yield $x->priv;
    };
  }
}

class Y extends X { private $priv = 'Y'; }
class Z extends X { private $priv = 'Z'; }

function test($x) {
  $f = $x->foo();
  foreach ($f($x) as $v) var_dump($v);
}

test(new X);
test(new Y);
test(new Z);
