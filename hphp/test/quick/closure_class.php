<?php

trait A {
  public function b() {
    return function() {
      return array(
        __CLASS__,
        get_class($this)
      );
    };
  }
}

class C {
  use A;
  public function d() {
    return function() {
      return array(
        __CLASS__,
        get_class($this)
      );
    };
  }
}

$c = new C;
$b = $c->b();
var_dump($b());
$d = $c->d();
var_dump($d());
