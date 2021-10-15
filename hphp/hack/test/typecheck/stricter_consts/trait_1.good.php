<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T {
  const int X = 1;
}

class C {
  use T;
}
