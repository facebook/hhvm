<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

namespace X\Y {
  class C implements \HH\FunctionAttribute {
    public function __construct(public int $i) {}
  }

  <<C(6)>>
  function j() {}

  <<__D(7)>>
  function k() {}

  function reflect () {
    $rj = new \ReflectionFunction("X\Y\j");
    \var_dump($rj->getAttributeClass(C::class)->i); // 6
    // Built in attributes don't get namespaced
    $rk = new \ReflectionFunction("X\Y\k");
    \var_dump($rk->getAttribute("__D")[0]); // 7
  }
}

namespace {
  use X\Y\C;

  class B implements HH\FunctionAttribute {
    public function __construct(public int $i) {}
  }

  <<B(4)>>
  function g() {}

  <<C(5)>>
  function h() {}

  function reflect () {
    $rg = new ReflectionFunction("g");
    \var_dump($rg->getAttributeClass(B::class)->i); // 4
    $rh = new ReflectionFunction("h");
    \var_dump($rh->getAttributeClass(C::class)->i); // 5
  }

  echo reflect();
  echo X\Y\reflect();
}
