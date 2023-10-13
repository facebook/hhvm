<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  abstract const type T;

  abstract public function get(): this::T;
}

function test(C $x): void {
  hh_show($x->get());
  function () use ($x) {
    hh_show($x->get());
  };
}
