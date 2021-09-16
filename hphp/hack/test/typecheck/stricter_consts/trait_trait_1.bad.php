<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// Error

trait T1 {
  const int X = 1;
}

trait T2 {
  const int X = 2;
}

class C {
  use T1, T2;
}
