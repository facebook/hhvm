<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__LateInit>> public $x;
}

class B extends A {
  public $x;
}

new B();
