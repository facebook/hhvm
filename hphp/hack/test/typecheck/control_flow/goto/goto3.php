<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B {}
class C {}
class D {}
class F {}

function h(bool $b, A $x): void {
  while ($b) {
    if ($b) {
      goto a;
    }
    $x = new B();
  }
  $x = new C();

  a:
  hh_show($x);
}

function expect<T>(T $_): void {}
