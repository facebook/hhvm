<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

enum class E: ExBox {
  // a single initializer is needed here.
   Box<string> A = new Box('bli'), 'some other arg';
}
