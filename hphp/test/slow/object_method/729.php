<?php

class A {
  var $a;
  function f() {
    var_dump($this->a);
  }
  function g() {
    $this->a = 100;
    call_user_func(array('self', 'f'));
  }
}
