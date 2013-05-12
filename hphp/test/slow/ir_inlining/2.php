<?php

class Test {
  public function foo($x) {
    return $x;
  }
}

function test4(Test $x) {
  $x->foo(12);
}

test4(new Test());
