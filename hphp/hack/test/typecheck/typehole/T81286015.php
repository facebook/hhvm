<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait MyTrait1 {
  public function testFun(): void {}
}
trait MyTrait2 {
  use MyTrait1;
}
class MyClass {
  use MyTrait1;
  use MyTrait2;
}

<<__EntryPoint>>
function main():void {
  $x = new MyClass();
  $x->testFun();
}
