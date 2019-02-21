<?hh // partial
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function bar(): void {}
}

function testit(): void {
  // Likewise this should not error in partial mode
  $f = $x ==> $x->foo();
  $f(new C());
}
