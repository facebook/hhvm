<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

<<file:__EnableUnstableFeatures(
  'enum_supertyping',
)>>

enum A : int {}
enum B : int  includes A {}

enum C : int as int {}
enum D : int as int includes C {}
enum E : int includes C {}
