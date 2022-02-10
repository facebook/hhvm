<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

enum E : int as int {
  V = 1;
}

trait T0 {
  public int $x = E::V;
}

trait T1 {
  public int $x = E::V;
}

trait T2 {
  public int $x = 1;
}

trait T3 {
  public int $x = 1;
}

class C {
  use T0;
  use T1;
  use T2;
  use T3;
}
