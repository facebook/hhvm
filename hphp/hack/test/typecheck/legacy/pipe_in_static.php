<?php

class C {
  public static function f($x) {
    return $x;
  }
  static $x = "foo" |> C::f($$);
}
