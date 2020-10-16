<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file: __EnableUnstableFeatures('enum_class')>>

class ExBox {}

class Box<T> extends ExBox {
  public function __construct(public T $data) {}
}

// Use a bare interface, not a class
enum class E: ExBox {
  A<Box<string>>(new Box('zuck'));
}
