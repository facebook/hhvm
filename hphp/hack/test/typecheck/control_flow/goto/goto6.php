<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B {}
class C {}
class D {}
class F {}

function fooo(bool $b, A $x): void {
  if ($b) {
    goto a;
  }
  $x = new B();
  $y ==> {
    $x = new C();
    if ($b) {
      goto a;
    }
    $x = new D();
    a:
    hh_show($x);
  };
  a:
  hh_show($x);
}

function expect<T>(T $_): void {}
