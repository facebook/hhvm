<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  <<__Const>> abstract public static int $p;
}

class B extends A {
  <<__Const>> public static arraykey $p = 1;
}
