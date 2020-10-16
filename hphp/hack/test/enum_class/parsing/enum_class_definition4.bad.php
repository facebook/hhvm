<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('enum_class')>>

interface ExBox {}

class Box<T> implements ExBox {
  public function __construct(public T $data) {}
}

enum class E: ExBox {
  // a single initializer is needed here.
  A<Box<string>>(new Box('bli'), 'some other arg');
}
