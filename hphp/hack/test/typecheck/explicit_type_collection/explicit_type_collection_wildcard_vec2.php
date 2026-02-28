<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
function foo(vec<int> $x): void {}
function bar(): void {
  foo(vec<_>["foo"]);
}
