<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Sealed(B::class)>>
enum A : int {}

enum B : int {
  use A;
}

enum C : int {
  use A;
}
