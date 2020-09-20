<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}
class B {}
class C {}
class D {}
class F {}

function f(A $x): void {
  goto a;
  $x = new B ();

  a:
  hh_show($x);
}

function expect<T>(T $_): void {}
