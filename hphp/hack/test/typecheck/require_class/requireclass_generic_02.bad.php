<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures('require_class')>>

trait MyTrait {
  require class C;
}

final class C<T> {
  use MyTrait;
}
