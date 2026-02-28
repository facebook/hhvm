<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C1 {}
class C2 {}

trait T {
  private ?C1 $c;
}

class C {
  use T;

  private ?C2 $c;
}
