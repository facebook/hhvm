<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C<reify T, Tu> {}
class D {}
class E<reify T> {}
class F<T> {}

function f(): void {
  new C<int, string>();
  new D<int>();
  new E();
  new F();
}
