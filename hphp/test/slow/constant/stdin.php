<?php
class A {
  public static function b() {
    return \STDIN;
  }
}
var_dump(A::b());
