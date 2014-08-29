<?php

class Foo {
  private $bar = 10;
  function foofunk($arg) {
    f($arg, "Test", $this);
  }
}

function f($arg1, $arg2, $arg3) {
  function g($arg4) {
    var_dump(xdebug_get_function_stack());
  }
  g($arg1 - 2);
}
(new Foo())->foofunk(10);
require("get_function_stack_helper.php");
eval("var_dump(xdebug_get_function_stack());");
