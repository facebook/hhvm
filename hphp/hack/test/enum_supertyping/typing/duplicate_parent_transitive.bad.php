<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum E: int as int {
  A = 0;
}

enum F: int {
  use E;
}

enum G: int {
  use F;
  A = 42;
}
