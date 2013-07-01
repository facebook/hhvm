<?php

define('FIZ', 32);

class X {
  const FOO = 1;
  const BAR = FIZ;
  const BAZ = FIZ;
  const BOO = FIZ;
  const BIZ = FIZ;
  const FIZ = FIZ;
}

function foo($a, $b) {
  var_dump($a, $b);
}

function f() { return FIZ; }

function test() {
  foo(f(), array(X::FOO, X::BAZ,
                 X::BAR, X::BAZ,
                 X::BOO, X::BIZ));
}

test();


