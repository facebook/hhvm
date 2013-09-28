<?php

if (isset($g)) {
  class X {
}
}
 else {
  class X {
    static function foo() {
}
    function bar() {
      X::foo(1,2,3);
    }
  }
}
