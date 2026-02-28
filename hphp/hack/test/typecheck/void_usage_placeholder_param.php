<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C1 {}
class C2 {}

function test_ok(C1 $_, C2 $_): void {}

function test_improper_usage(C1 $c1, C2 $_): void {
  test_ok($c1, $_);
}
