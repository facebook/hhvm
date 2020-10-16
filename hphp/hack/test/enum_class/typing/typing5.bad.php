<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file: __EnableUnstableFeatures('enum_class')>>

interface ExBox<T> {}

class Box<T> implements ExBox<T> {
  public function __construct(public T $data) {}
}

// only interfaces without parameters are allowed
enum class F : ExBox<T> {
  A<Box<string>>(new Box('zuck'));
}
