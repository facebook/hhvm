<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait A {
  <<__Const>> abstract public static arraykey $p;
}

class B {
  use A;
  <<__Const>> public static int $p = 1;
}
