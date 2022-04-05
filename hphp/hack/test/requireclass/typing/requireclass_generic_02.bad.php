<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait MyTrait {
  require class C;
}

class C<T> {
  use MyTrait;
}
