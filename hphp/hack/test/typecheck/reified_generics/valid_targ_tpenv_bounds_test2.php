<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}

class Test<reify Tc as A> {
  public function f<Tf as Tc>(Test<Tf> $t): void {}

  public function g<reify Tg as A>(Test<Tg> $t): void {}

  public function h(Test<Tc> $t, Test<A> $u): void {}
}
