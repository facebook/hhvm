<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Good: X is abstract in I

interface I {
  abstract const int X;
}

trait T {
  const int X = 1;
}

class D implements I {
  use T;
}
