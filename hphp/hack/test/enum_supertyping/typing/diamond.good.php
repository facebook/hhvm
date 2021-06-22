<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum E1 : int {
  A = 1;
}

enum E2 : int {
  use E1;
}

enum E3 : int {
  use E1;
}

enum E : int {
  use E2;
  use E3;
}
