<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  <<__Const>> public abstract arraykey $p;
}

class B extends A {
  <<__Const>> public int $p = 1;
}
