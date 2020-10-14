<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures(
  'enum_supertyping',
)>>

enum E : int {}

enum F : int includes E {}
