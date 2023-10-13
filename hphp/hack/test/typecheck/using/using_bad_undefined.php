<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():void { }
}
class Handle implements IDisposable {
  public function __dispose():void { }
  public function mescape(<<__AcceptDisposable>> IDisposable $x):void { }
  public function foo():void { }
}
class AsyncHandle implements IAsyncDisposable {
  public async function __disposeAsync():Awaitable<void> { }
  public function bar():void { }
}

function escape(<<__AcceptDisposable>> Handle $h):void {
  var_dump($h);
}

function testit():void {
  using ($x = new Handle()) {
    $x->foo();
  }
  // Shouldn't be defined now
  $x->foo();
}

function testit2(bool $b):void {
  // Likewise with conditionals
  if ($b) {
    $x = new C();
  } else {
    using ($x = new Handle()) {
      $x->foo();
    }
  }
  // Shouldn't be defined now
  $x->foo();
}
