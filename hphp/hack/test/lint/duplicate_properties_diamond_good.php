<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait T0 {
  public int $y;
}

trait T1 {
  use T0;
}

class C {
   use T1;
   use T2;
}
