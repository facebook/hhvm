<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

trait MyTrait {
}

enum class E: ExBox {
  // Cannot use a trait inside an enum class
  use MyTrait;
   Box<string> A = new Box('bli');
}
