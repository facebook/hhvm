<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class D {
  const int V = 1;
}

trait T0 {
  public int $x = D::V;
}

trait T1 {
  public int $x = D::V;
}

class C {
  use T0;
  use T1;
}
