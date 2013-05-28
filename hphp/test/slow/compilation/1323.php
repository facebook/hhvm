<?php

class Y {
  function bar() {
}
}
class X {
  function foo() {
    $x = $this;
    if ($this instanceof y) {
      $this->bar();
    }
    return $x;
  }
}
