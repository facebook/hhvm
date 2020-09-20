<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B {}
class C {}
class D {}
class F {}

function hh(bool $b, A $x): void {
  while ($b) {
    if ($b) {
      goto a;
    }
    $x = new B();
    if ($b) {
      goto b;
    } else if ($b) {
      $x = new F();
      goto a;
    }
  }
  $x = new C();

  a:
  hh_show($x);
  $x = new D();
  b:
  hh_show($x);
}

function expect<T>(T $_): void {}
