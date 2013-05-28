<?php

class X {
  static function foo() {
    var_dump(__METHOD__, get_called_class());
  }
  function bar() {
    var_dump(__METHOD__, get_called_class());
  }
}
class Y extends X {
}
class Z extends X {
  static function foo() {
    var_dump(__METHOD__, get_called_class());
  }
}
function test($x, $o) {
  $x->getMethod('foo')->invoke($o);
  $x->getMethod('foo')->invoke(null);
  $x->getMethod('bar')->invoke($o);
}
test(new ReflectionClass('X'), new X);
test(new ReflectionClass('Y'), new X);
test(new ReflectionClass('X'), new X);
test(new ReflectionClass('Z'), new Z);
call_user_func(array(new Y, 'X::foo'));
call_user_func(array(new Z, 'X::foo'));
