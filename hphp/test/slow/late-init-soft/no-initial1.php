<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__SoftLateInit>> public $x1 = 123;
}

new A();
