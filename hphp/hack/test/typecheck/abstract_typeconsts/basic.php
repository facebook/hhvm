<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  abstract const type T as arraykey = arraykey;
}

class A1 extends A {}

class A2 extends A {
  const type T = string;
}
