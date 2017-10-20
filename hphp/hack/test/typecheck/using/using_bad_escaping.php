<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose():void { }
  public function foo():void { }
}
class AsyncHandle implements IAsyncDisposable {
  public async function __disposeAsync():Awaitable<void> { }
  public function bar():void { }
}

function expect_mixed(mixed $m):void { }

function test_it():mixed {
  using ($x = new Handle()) {
    $y = $x;
  }
  using ($z = new Handle()) {
    expect_mixed($z);
  }
  using ($w = new Handle()) {
    return $w;
  }
  using ($q = new Handle()) {
    using ($q) { }
  }
  using ($r = new Handle()) {
    using ($m = $r) { }
  }
}
