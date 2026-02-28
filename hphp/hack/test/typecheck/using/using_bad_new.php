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

function test_it(): void {
  $x = new Handle();

  // This is legal
  using ($x = new Handle()) {
    // But this is not
    $w = new Handle();
  }

  $z = new AsyncHandle();
}
