<?hh
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
function expect_handle(<<__AcceptDisposable>> Handle $h):void {
  // Illegal because it escapes to a function not marked with the attribute
  expect_mixed($h);
}
function expect_handle_good(<<__AcceptDisposable>> Handle $h):void {
  $h->foo();
}
function expect_handle_bad(<<__AcceptDisposable>> Handle $h):mixed {
  return $h;
}

function test_it():mixed {
  using ($x = new Handle()) {
    $y = $x;
  }
  using ($z = new Handle()) {
    expect_mixed($z);
  }
  using ($q = new Handle()) {
    using ($q) { }
  }
  using ($r = new Handle()) {
    using ($m = $r) { }
  }
  using ($s = new Handle()) {
    // OK, but look at body of function
    expect_handle($s);
  }
  using ($w = new Handle()) {
    return $w;
  }
}
