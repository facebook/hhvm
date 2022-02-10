<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

// HHVM fatals because it cannot prove that $x, inherited via two different
// paths, is always initialised to 1.

enum E : int as int {
  V = 1;
}

trait T0 {
  public int $x = E::V+1;
}

trait T1 {
  use T0;
}

class C {
  use T0;
  use T1;
}
