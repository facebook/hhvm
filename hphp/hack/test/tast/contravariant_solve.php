<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

interface I<-T> {}

class E<T> implements I<T> {}

function f<T>(I<T> $_, I<T> $_): I<T> {
  throw new Exception();
}

function g<T>(T $_): E<T> {
  throw new Exception();
}

function test(): void {
  $_ = f(g(1), g(2));
}
