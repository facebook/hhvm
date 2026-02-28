<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  <<__Const>> abstract static public int $a = 5;
  abstract const int C = 5;
}
class B {
  <<__Const>> static public int $b;
  const int C;
}
