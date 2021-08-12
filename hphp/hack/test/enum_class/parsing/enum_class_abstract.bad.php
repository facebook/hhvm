<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.
<<file:__EnableUnstableFeatures('abstract_enum_class')>>

final enum class E : mixed {}

enum class E : mixed {
  int A;
}

abstract enum class G : mixed {
  abstract int A = 42;
}

enum class G : mixed {
  final int A = 42;
}
