<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data)[] {}
}

// only types without free parameters are allowed
enum class E : Box<T> {
   Box<string> A = new Box('zuck');
}
