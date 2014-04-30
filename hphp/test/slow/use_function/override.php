<?php

namespace foo {
  function bar() {
    return __FUNCTION__;
  }
}

namespace {
  function bar() {
    return __FUNCTION__;
  }

  var_dump(bar());

  use function foo\bar as bar;

  var_dump(bar());
}
