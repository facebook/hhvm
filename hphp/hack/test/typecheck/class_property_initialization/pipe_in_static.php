<?php

class C {
  static function f($x) {
    return $x;
  }
  static $x = "foo" |> C::f($$);
}
