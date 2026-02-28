<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose():void { }
  public function foo():void { }
}

function test_it():void {
  using ($v = new Handle()) {
    $f = () ==> $v->foo();
    $g = function():void use($v) { $v->foo(); };
  }
  // Bad news
  ($f)();
  ($g)();
}
