<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A<T> {}
class B<T> {}
class C<-T> {}
class D {}

function make_a<T>(): A<T> {
  return new A();
}

function a_to_b<T>(
  A<T> $_,
): B<T> {
  return new B();
}

function b_to_c<T>(
  B<T> $_,
): C<T> {
  return new C();
}

function a_to_c<T>(
  A<T> $_,
): C<T> {
  return new C();
}

function test1(): C<D> {
  return a_to_c(make_a());
}

function test2(): C<D> {
  return b_to_c(a_to_b(make_a()));
}
