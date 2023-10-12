<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __construct(private string $name) { }
  public function __dispose():void { }
  public function foo():void { }
  <<__ReturnDisposable>>
  public function new_handle():Handle {
    return new Handle("whatever");
  }
  <<__ReturnDisposable>>
  public function bad_alias():this {
    return $this;
  }
  <<__ReturnDisposable>>
  public function bad_escape(<<__AcceptDisposable>> IDisposable $x):IDisposable {
    return $x;
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
    // This is illegal
    $w = $x->new_handle();
  }
  // This should be illegal
  $z = make_handle(true);
  // And this!
  $f = make_handle<>;
  $x = $f(false);
}
