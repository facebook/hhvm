<?php

function wrapper($a) {
  if ($a) {
    class C {
      private static $v;
      public static function f() {
        return self::$v;
      }
    }
  }
}
class C2 {
  private static $v;
  public static function f() {
    return self::$v;
  }
}
function foo($a) {
  if ($a == 0) return is_callable(array('C', 'f'), null);
  return is_callable(array('C2', 'f'), null);
}
wrapper(false);
var_dump(foo(0));
var_dump(foo(1));
if (class_exists('C')) var_dump('yes');
 else var_dump('no');
