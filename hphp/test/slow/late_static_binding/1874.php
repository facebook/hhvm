<?php

class B {
  public static $a = 100;
  static function f() {
    var_dump(static::$a);
  }
}
class C extends B {
  public static $a = 1000;
}
call_user_func(array('C', 'f'));
