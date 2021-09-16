<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// No errors

abstract class A {
  const type T = float;
}
trait Tr {
  abstract const type T = int;
}
class C extends A {
  use Tr;

  public function test(this::T $t): void {
    hh_show($t); // expecting float
  }
}
