<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public arraykey $p = 5;
}

class B extends A {
  public int $p = 5; // error
}
