<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

trait T {}

class C {
  use T;
}

function foo(): void {
  $_ = (): T ==> new C();
}
