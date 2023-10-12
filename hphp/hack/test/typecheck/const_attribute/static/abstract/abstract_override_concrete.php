<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  <<__Const>> public static string $x = "concrete";
}

abstract class D extends C {
  <<__Const>> abstract public static string $x; // illegal
}
