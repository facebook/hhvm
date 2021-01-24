<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum E1: int as int {
  A = 0;
}

enum E2: int as int {
  A = 1;
}

enum F1: int {
  use E1;
  use E2;
  A = 2;
}

enum F2: int {
  use E1, E2;
  A = 2;
}
