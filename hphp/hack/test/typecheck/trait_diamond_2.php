<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// OK: since testFun is abstract, multiple imports of abstract method are tolerated

trait MyTrait1 {
  abstract public function testFun(): void;
}
trait MyTrait2 {
  use MyTrait1;
}

trait MyTrait3 {
  use MyTrait1;
  use MyTrait2;
}
