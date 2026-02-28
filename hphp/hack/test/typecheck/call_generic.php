<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test1<T as (function(): void)>(T $f): void {
  $f();
}

class C {}

function test2<T as (function(): void) as C>(T $f): void {
  $f();
}
