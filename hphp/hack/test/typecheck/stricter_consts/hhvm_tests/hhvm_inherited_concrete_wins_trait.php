<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class A {
  abstract const type T = int;
}
trait Tr {
  const type T = string;
}
class C extends A {
  use Tr;

  public function test(this::T $t) : void {
    hh_show($t); // expecting string
  }
}
