<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public $x;
}

class B extends A {
  <<__SoftLateInit>> public $x;
}

new B();
