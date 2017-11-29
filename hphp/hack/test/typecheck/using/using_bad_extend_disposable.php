<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class Baz { static public mixed $x; }
class Moo {
  public function storeThis(): void {
    Baz::$x = $this;
  }
}

interface I { }
// It's ok to implement another interface
final class Foo implements IDisposable, I {
  public function __dispose():void { }
  use Bar; // this should error
}
final class Boo extends Moo implements IDisposable {
  public function __dispose():void { }
}

trait Bar {
  public function storeThis(): void {
    Baz::$x = $this;
  }
}

function main(): void {
  using $foo = new Foo();
  $foo->storeThis(); // very bad
}
