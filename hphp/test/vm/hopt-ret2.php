<?php

class C {
  public $y=5;
  function foo($x) {
    return $x + $this->y;
  }
}

$o = new C;
echo $o->foo(3) . "\n";
