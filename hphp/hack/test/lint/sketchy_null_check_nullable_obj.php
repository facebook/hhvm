<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}

function noop(): void {}

function test(?A $x): void {
  if ($x) {
    noop();
  }
}
