<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function f(): void {}

function test(int $x): void {
  if ($x === null) {
    f();
  }
}
