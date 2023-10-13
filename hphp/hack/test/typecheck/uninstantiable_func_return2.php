<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

trait T {}

class C {
  use T;
}

function foo(): T {
  return new C();
}
