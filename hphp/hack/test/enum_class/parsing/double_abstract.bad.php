<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('abstract_enum_class')>>


abstract abstract enum class C : mixed {}

abstract enum class D : mixed {
  abstract abstract int X;
}
