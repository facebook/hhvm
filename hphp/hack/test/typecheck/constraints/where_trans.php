<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}

class Test<Tc as A> {
  public function f<Tf as Tc>(Test<Tf> $t): void {}

  public function g<Tg>(Test<Tg> $t): void where Tg as Tc {}
}
