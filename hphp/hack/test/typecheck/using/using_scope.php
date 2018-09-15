<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function foo():int { return 4; }
}
class Handle implements IDisposable {
  public function __dispose():void { }
  public function foo():int { return 4; }
  public function mescape(<<__AcceptDisposable>> IDisposable $x):void { }
}
class AsyncHandle implements IAsyncDisposable {
  public async function __disposeAsync():Awaitable<void> { }
  public function bar():void { }
}

function testit(bool $b):int {
  using ($x = new Handle()) {
    if ($b) {
      using ($y = new Handle()) {
        $z = $y->foo();
      }
      $z++;
      $a = $z;
    }
    else {
      $a = 52;
    }
  }
  return $a;
}

function global_lol(bool $condition): string {
  if ($condition) {
    using (new Handle())
    {
      $lol = "foo";
    }
  } else {
    $lol = "bar";
  }
  return $lol;
}
function global_lolly(bool $condition): string {
  if ($condition) {
    {
      $boo = "foo";
    }
  } else {
    $boo = "bar";
  }
  return $boo;
}
