<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  <<__Const>> abstract public static arraykey $p;
}

class B extends A {
  <<__Const>> public static int $p = 1;
}
