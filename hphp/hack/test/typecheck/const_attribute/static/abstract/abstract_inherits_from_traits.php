<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait A {
  <<__Const>> abstract static public int $p;
}

trait B {
  <<__Const>> abstract static public num $p;
}

abstract class C {
  use A, B;
}
