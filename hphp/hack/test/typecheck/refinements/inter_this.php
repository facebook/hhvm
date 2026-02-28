<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {}
interface I {
  public function takesThis(this $x): void;
}

function test(C $c): void {
  if ($c is I) {
    $c->takesThis($c);
  }
}
