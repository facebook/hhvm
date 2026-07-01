<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {}

function test<T>(class<T> $c): C {
  return new $c();
}
