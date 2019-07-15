<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I1 {}

class A0 {}

class A extends A0 implements I1 {}

class B implements I1 {}

class C extends A {}

function g() : void {
  $b = new B();
}

function f(C $c): void  {
  g();
}
