<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  abstract const type T as arraykey = arraykey;
}

class B extends A {}

class C extends B {
  const type T = string;
}
