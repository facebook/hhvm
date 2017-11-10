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
    $x->foo();
  }

  // Should be able to re-use $x as it's now unset
  $x = 3;

  // Even here this should be ok
  using ($x = new Handle(), $y = new Handle()) {
    $x->foo(); $y->foo();
    // Nested
    using ($z = new Handle()) {
      $z->foo();
    }
    $z = 'a';
  }

  // Now with function scope
  using $x = new Handle();
  $x->foo();

  await using ($a = new AsyncHandle()) {
    $a->bar();
  }
  $a = 23;
  await using $a = new AsyncHandle();
  $a->bar();
  using (new Handle()) {
    // Still in scope
    $a->bar();
  }
}
