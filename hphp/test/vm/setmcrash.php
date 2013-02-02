<?php

class X {
  private $foo;
  function foo(&$b) {
    $this->foo = $b;
  }
}

$x = new X;
$t = null;
$x->foo($t);

