<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  abstract const type T as num = int;
}

class AA extends A {}

abstract class B extends A {
  abstract const type T as num = float;
}

class BB extends B {}
