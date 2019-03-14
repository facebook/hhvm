<?php

class X {
  function bar() {
    var_dump(static::class);
  }
}
class Y extends X {
  function foo() {
    call_user_func(array('SELF', 'bar'));
    call_user_func(array('PARENT', 'bar'));
    call_user_func(array('STATIC', 'bar'));
    call_user_func('SELF::bar');
    call_user_func('PARENT::bar');
    call_user_func('STATIC::bar');
  }
}

<<__EntryPoint>>
function main_1886() {
;
Y::foo();
}
