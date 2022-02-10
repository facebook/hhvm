<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T0 {
  public int $y;
  public int $z;
}

trait T1 {
  use T0;
  public int $x = 3;
  public int $y;
}

trait T2 {
  public int $x = 2+1;
}

class C {
   use T1;
   use T2;
  public int $z = 3;
}
