<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public static arraykey $p = 5;
}

class B extends A {
  public static int $p = 5; // error
}
