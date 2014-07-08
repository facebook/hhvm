<?php

require('call_function_helper.php');

function test1() {
  var_dump(xdebug_call_function());
  test2();
}

function test2() {
  var_dump(xdebug_call_function());
}


class Foo {
  function test3() {
    test2();
  }
}

var_dump(xdebug_call_function());
test1();
(new Foo())->test3();
