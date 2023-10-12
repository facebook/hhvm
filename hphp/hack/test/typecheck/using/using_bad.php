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

async function testit():Awaitable<void> {
  using ($x = new Handle()) {
    // Should not be able to assign $x as it's a using variable
    $x = 3;
  }

  // Repeated variables in same using clause
  using ($x = new Handle(), $x = new Handle()) {
    $x->foo();
  }

  // Now with function scope
  using $x = new Handle();
  using $x = new Handle();

  // Should have await here
  using ($a = new AsyncHandle()) {
    $a->bar();
  }
  // Shouldn't have await here
  await using $b = new Handle();
}
