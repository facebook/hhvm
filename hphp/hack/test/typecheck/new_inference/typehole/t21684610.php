<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I1<T> {}
interface I2<T> {}
final class A implements I2<float> {}

function f<T, Ti as I1<T>>(I2<T> $_, Ti $_): void {}

function a(): A {
  return new A();
}

function test1(): void {
  f(a(), 1); // this passes!!!
}
/*
function test2(A $a): void {
  f($a, 1); // this fails (int is not compatible with I1)
}
*/
