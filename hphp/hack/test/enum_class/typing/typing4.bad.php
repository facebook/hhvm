<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file: __EnableUnstableFeatures('enum_class')>>

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

// only interfaces without parameters are allowed
enum class E : Box<T> {
  A<Box<string>>(new Box('zuck'));
}
