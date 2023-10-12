<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo(): void {}
}
function test_callback(): void {
  $f = $r ==> {
    if ($r === 3) {
      return;
    } else {
      return new C();
    }
  };
  $x = $f(3);
}
