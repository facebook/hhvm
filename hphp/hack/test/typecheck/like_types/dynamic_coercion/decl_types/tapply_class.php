<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function dyn(): dynamic { return 4; }

class C {}
class E<T> {}
class R<reify T> {}

function concrete(): C {
  return dyn(); // ok
}

function erased(): E<int> {
  return dyn(); // error
}

function reified(): R<int> {
  return dyn(); // ok
}
