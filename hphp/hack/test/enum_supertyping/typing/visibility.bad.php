<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum A : int {}

enum B : int as int {
  use A;
}
