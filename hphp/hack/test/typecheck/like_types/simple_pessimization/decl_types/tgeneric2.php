<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class E<T> {}
class R<reify T> {}

class X<Te, reify Tr> {
  public function f(E<Te> $x): void {
    hh_show($x);
  }

  // Tr by itself becomes ~Tr because it is not enforceable,
  // but if it is in a reified position it stays trusted
  public function g(R<Tr> $x): void {
    hh_show($x);
  }

  public function h(R<(Tr, Tr)> $x): void {
    hh_show($x);
  }
}
