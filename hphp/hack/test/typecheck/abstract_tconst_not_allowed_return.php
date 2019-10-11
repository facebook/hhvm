<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  abstract const type T;
  abstract public function foo(): this::T;
}

function f(C $c): C::T {
  return $c->foo();
}
