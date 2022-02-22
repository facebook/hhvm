<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface ExBox<T> {}

class Box<T> implements ExBox<T> {
  public function __construct(public T $data)[] {}
}

// only types without parameters are allowed
enum class F : ExBox<T> {
   Box<string> A = new Box('zuck');
}

// dynamic is not allwed
enum class D : dynamic {
}
