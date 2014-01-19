<?php

$a = 'self';
class A {
  public static function f($value) {
    $filter = 'g';
    return call_user_func(array($GLOBALS['a'], $filter), $value);
  }
  public static function g($value) {
    return $value;
  }
}
var_dump(A::f('test'));
