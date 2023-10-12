<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}

class Test<reify Tc as A> {
  public function f<Tf as Tc>(): Test<Tf> {
    return new Test<Tf>();
  }

  public function g<reify Tg as A>(): Test<Tg> {
    return new Test<Tg>();
  }
}
