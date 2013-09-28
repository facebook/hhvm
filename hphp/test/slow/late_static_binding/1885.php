<?php

class A {
  public static function foo() {
    var_dump(get_called_class());
  }
  public static function bar() {
    $x = new B;
    $x->fiz(self::foo());
  }
  function fiz($x) {
}
}
class B extends A {
}
$array = array('foo');
array_map('B::foo', $array);
call_user_func('B::foo');
call_user_func(array('B', 'foo'));
A::bar();
