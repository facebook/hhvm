<?php

class foo {
  static function ioo($y, &$x) {
    return new self(1);
  }
  function __construct($a, $b) {
}
}
function t() {
  $x = 1;
  foo::ioo($x, $y);
}
t();
