<?hh
// Copyright 2004-present Facebook. All Rights Reserved.
function foo(keyset<int> $x): void {}
function bar(): void {
  $x = keyset<arraykey>[];
  hh_show($x);
  foo($x);
}
