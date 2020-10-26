<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<__ConsistentConstruct>>
abstract class A {
  abstract const type TP;
  public function __construct(public this::TP $item) { }
}
class C extends A {
  const type TP = ?int;
}

class D extends A {
  const type TP = ?bool;
}

function testit(bool $b):void {
  $cn = $b ? C::class : D::class;
  $a = new $cn(null);
}
