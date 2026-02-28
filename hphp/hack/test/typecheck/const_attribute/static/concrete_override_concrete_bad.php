<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__Const>> public static int $p = 5;
}

class B extends A {
  <<__Const>> public static arraykey $p = 55;
}
