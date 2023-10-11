<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

trait A {
  <<__Const>> abstract public static int $p;
}

class B {
  use A;
  <<__Const>> public static arraykey $p = 1;
}
