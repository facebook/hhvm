<?php

class Foo {
  static $x;
  static $y;

  function x() {
    self::$x++;
    self::$y += 12;
  }

  function go() {
    $this->x();
    var_dump(self::$x);
    var_dump(self::$y);
  }
}

(new Foo)->go();
