<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum E: int as int {
  A = 0;
}

enum F: int as int {
  use E;
}

enum G: int as int {
  A = 1;
}

enum H: int {
  use F;
  use G;
}
