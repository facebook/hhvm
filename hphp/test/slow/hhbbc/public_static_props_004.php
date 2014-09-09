<?php

class B {
  static $x = 'ok';
}

final class Y extends B { // nooverride
  static $x = 2;
}

function set(Y $y) {
  $y::$x = 42;
}

B::$x = 'another string';

function x() {
  set(new Y);
  var_dump(is_int(Y::$x));
  var_dump(is_string(B::$x));
}

x();
