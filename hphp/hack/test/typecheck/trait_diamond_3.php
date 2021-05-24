<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// OK: trait A is used only via the path MySubClass -> B -> A

trait A {
  public function testFun(): void {}
}

class MyClass {
  use A;
}

trait B {
  use A;
}

trait C {
  require extends MyClass;
}

class MySubClass extends MyClass {
  use B;
  use C;
}
