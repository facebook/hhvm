<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __construct(private string $name) { }
  public function __dispose():void { }
  public function foo():void { }
  public function mescape(<<__AcceptDisposable>> IDisposable $x):void { }
  <<__ReturnDisposable>>
  public function make():Handle {
    return make_handle(false);
  }
  <<__ReturnDisposable>>
  public static function smake():Handle {
    return make_handle(true);
  }
  <<__ReturnDisposable>>
  public function another():IDisposable {
    return self::smake();
  }
}
class AsyncHandle implements IAsyncDisposable {
  public async function __disposeAsync():Awaitable<void> { }
  public function bar():void { }
}
<<__ReturnDisposable>>
function make_handle(bool $b):Handle {
  if ($b) return new Handle("yes");
  else return new Handle("no");
}

function testit():void {
  using ($x = make_handle(false)) {
    $x->foo();
  }
  using ($y = Handle::smake()) {
    using ($z = $y->make()) {
      $z->foo();
    }
  }
}
