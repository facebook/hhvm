<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__SoftLateInit>> public $x;
}

class B extends A {
  <<__LateInit>> public $x;
}

new B();
