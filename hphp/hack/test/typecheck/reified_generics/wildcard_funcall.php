<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A<reify T> {}

function f<T>(T $t): void {}

function test(): void {
  f<_>(new A<int>());
  f<A<_>>(new A<int>());
  f<A<string>>(new A<int>());

  f<_>(new A<A<int>>());
  f<A<_>>(new A<A<int>>());
  f<A<A<_>>>(new A<A<int>>());
  f<A<A<string>>>(new A<A<int>>());
}
