<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__SoftLateInit>> public static $x1 = 123;
}

new A();
