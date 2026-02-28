<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose():void { }
  public function foo():void { }
  public function mescape(<<__AcceptDisposable>> IDisposable $x):void { }
}
class AsyncHandle implements IAsyncDisposable {
  public async function __disposeAsync():Awaitable<void> { }
  public function bar():void { }
}
function expect_string(string $s):void { }
class Another implements IDisposable {
  public function __construct(private ?string $str) {
    $s = $this->str;
    $this_str = $s;
    printf("I got a string: %s\n", $this->str);
  }
  public function copyStr(<<__AcceptDisposable>> Another $x):void {
    $this->str = $x->str;
    if ($this->str !== null) {
      // fake member
      expect_string($this->str);
    }
  }
  public function __dispose(): void { }
}
function escape(<<__AcceptDisposable>> Handle $h):void {
  var_dump($h);
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
      // This is legal because var_dump is marked as <<__AcceptDisposable>>
      var_dump($x);
      var_dump($z);
      $z->foo();
    }
    $z = 'a';
  }

  // Now with function scope
  using $x = new Handle();
  $x->foo();
  // Legal because of attribute
  escape($x);
  $x->mescape($x);

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
