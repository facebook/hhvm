<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  abstract const type T as num = int;
}

class B extends A {}

abstract class C extends B {
  abstract const type T as num = float;
}

class D extends C {}
