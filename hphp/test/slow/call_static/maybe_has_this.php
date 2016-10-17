<?php

__hhvm_intrinsics\disable_inlining('X::foo');
__hhvm_intrinsics\disable_inlining('X::bar');

class X {
    function foo($x) {
      if ($x instanceof X) {
        $x::bar();
      }
    }
    function bar() {
        if (isset($this)) var_dump($this);
        else var_dump(__METHOD__);
    }
}

class Y extends X {}
class Z extends Y {}

function test($x, $y) {
  $x->foo($y);
}

function main() {
  for ($i = 0; $i < 10; $i++) {
    test(new X, new Y);
    test(new Y, new Y);
    test(new Z, new Y);
    test(new Y, new Z);
  }
}

main();
