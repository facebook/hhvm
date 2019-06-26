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

function erased_dynamic(): E<dynamic> {
  return dyn(); // ok
}

function reified_dynamic(): R<dynamic> /* currently banned */ {
  return dyn(); // ok
}

function erased_like(): E<~int> {
  return dyn(); // ok
}

function reified_like(): R<~int> {
  return dyn(); // ok
}
