<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T {
  const int X = 0;
}

final class A {
  use T;
  const int X = 1;
}
