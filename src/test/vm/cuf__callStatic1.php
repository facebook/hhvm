<?php

class A {
  public static function __callStatic($f, $a) {
    echo "In A::__callStatic\n";
  }
  public function entry() {
    call_user_func('A::foo');
    call_user_func(array('A', 'foo'));
  }
  static public function static_entry() {
    call_user_func('A::foo');
    call_user_func(array('A', 'foo'));
  }
}


function caller() {
  $obj2 = new A;
  call_user_func('A::foo');
  call_user_func(array('A', 'foo'));
  call_user_func(array($obj2, 'foo')); // raises warning; no __callStatic call
}

function main() {
  $obja = new A;

  $obja->entry();
  A::static_entry();

  call_user_func('A::foo');
  call_user_func(array('A', 'foo'));
  call_user_func(array($obja, 'foo')); // raises warning; no call to __callStatic
  caller();
}
main();

