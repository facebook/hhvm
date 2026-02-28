<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Handle implements IDisposable {
  public function __dispose(): void {}
  public function foo(): void {}
}

class Base {
  public function foo(<<__AcceptDisposable>>mixed $h): void {}
  public function bar(mixed $h): void {}
}
class BadDerived1 extends Base {
  public function foo(mixed $h): void {}
  public function bar(mixed $h): void {}
}
class BadDerived2 extends Base {
  public function foo(<<__AcceptDisposable>>mixed $h): void {}
  public function bar(<<__AcceptDisposable>>mixed $h): void {}
}
class GoodDerived extends Base {
  public function foo(<<__AcceptDisposable>>mixed $h): void {}
  public function bar(mixed $h): void {}
}
class RBase {
  <<__ReturnDisposable>>
  public function foo(): void {}
  public function bar(): void {}
}
class RBadDerived1 extends RBase {
  public function foo(): void {}
  public function bar(): void {}
}
class RBadDerived2 extends RBase {
  <<__ReturnDisposable>>
  public function foo(): void {}
  <<__ReturnDisposable>>
  public function bar(): void {}
}
class RGoodDerived extends RBase {
  <<__ReturnDisposable>>
  public function foo(): void {}
  public function bar(): void {}
}
