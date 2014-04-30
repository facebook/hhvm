<?php

namespace foo {
  function bar() {
    return __FUNCTION__;
  }
}

namespace {
  use function foo\bar as foobar;
  use function foobar as foo_bar;

  var_dump(foobar());
  var_dump(foo_bar());
}
