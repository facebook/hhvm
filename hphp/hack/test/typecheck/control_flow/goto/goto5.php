<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B {}
class C {}
class D {}
class F {}

enum E: int {
  A = 0;
  B = 1;
  C = 2;
}

function foo(E $e, A $x): void {
  switch ($e) {
    case E::A:
      $x = new A();
      goto a;
    case E::B:
      $x = new B();
      goto b;
    case E::C:
      $x = new C();
      goto c;
  }

  a:
  expect<A>($x);
  return;
  b:
  expect<B>($x);
  return;
  c:
  expect<C>($x);
  hh_show($x); // just making sure we actually reach this point
  return;
}

function expect<T>(T $_): void {}
