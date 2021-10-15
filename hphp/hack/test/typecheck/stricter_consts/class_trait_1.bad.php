<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Error

class C {
  const int X = 2;
}

trait T {
  const int X = 1;
}

class D extends C {
  use T;
}
