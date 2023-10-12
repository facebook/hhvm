<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {}

class B extends A {}

function f(): int {
  $x = Pair<A, B>{new B(), new B()};
  return 0;
}
