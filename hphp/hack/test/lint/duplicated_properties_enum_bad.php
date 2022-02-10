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

class C {
  use T0;
  use T1;
}
