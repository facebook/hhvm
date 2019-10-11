<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class C {
  abstract const type T;
}

function f<T>(): void {}

function test(): void {
  f<C::T>();
}
