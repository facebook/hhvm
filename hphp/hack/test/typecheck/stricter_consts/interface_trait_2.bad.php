<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface I {
  const int X = 2;
}

trait T {
  const int X = 1;
}

class C implements I {
  use T;
}
