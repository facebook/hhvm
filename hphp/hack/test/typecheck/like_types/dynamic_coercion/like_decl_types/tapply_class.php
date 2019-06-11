<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {}
class E<T> {}
class R<reify T> {}

function concrete(~C $i): C {
  return $i; // ok
}

function erased(~E<int> $i): E<int> {
  return $i; // error
}

function reified(~R<int> $i): R<int> {
  return $i; // ok
}
