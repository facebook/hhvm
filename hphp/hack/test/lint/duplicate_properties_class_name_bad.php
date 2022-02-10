<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class D {
}

trait T0 {
  public int $x = D::class;
}

trait T1 {
  public int $x = D::class;
}

class C {
  use T0;
  use T1;
}
