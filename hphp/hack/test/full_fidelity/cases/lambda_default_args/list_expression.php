<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const string S = "hello";
  public string $s = "${A::S}";
  const string S1 = "${A::S}";

  public function test(): void {
    $_ = (string $p = "${A::S}"): void ==> {};
  }
}
