<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function f<T super int>(): T {
  return 0;
}

function g<T as string>(T $_): void {}

function test(): void {
  $x = f();
  g($x);
}
