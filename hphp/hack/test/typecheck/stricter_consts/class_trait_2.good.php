<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

abstract class A {
  abstract const int X;
}

trait T {
  const int X = 1;
}

class C extends A {
  use T;
}
