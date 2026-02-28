<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose():void { }
  public function foo():mixed {
    // Shouldn't be permitted
    return $this;
  }
  public function bar():void {
    expect_mixed($this);
  }
}
class AsyncHandle implements IAsyncDisposable {
  public async function __disposeAsync():Awaitable<void> { }
  public function bar():void {
    expect_async_handle_good($this);
  }
  public function foo():void {
    expect_mixed($this);
  }
}
trait MyTrait {
  require extends Handle;
  public function foo(): void {
    expect_mixed($this);
  }
}

function expect_mixed(mixed $m):void { }
function expect_handle_good(<<__AcceptDisposable>> Handle $h):void {
  $h->foo();
}
function expect_async_handle_good(<<__AcceptDisposable>> AsyncHandle $h):void {
}
