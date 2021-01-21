<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function expectReified<reify T>(mixed $x):void {
}
class B { }
final class D
{
  const type TQ as B = B;

  public function test():void {
    expectReified<this::TQ>(new B());
  }
}
