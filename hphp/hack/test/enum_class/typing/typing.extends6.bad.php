<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file: __EnableUnstableFeatures('enum_supertyping')>>

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

class IBox extends Box<int> {
  public function add(int $x): void {
    $this->data = $this->data + $x;
  }
}

enum class E: ExBox {
   Box<string> A = new Box('zuck');
   IBox B = new IBox(42);
}

enum NormalEnum : int {
  use E;
  Z = 42;
}
