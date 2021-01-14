<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

enum class E: ExBox {
  A = new Box('bli');
}
