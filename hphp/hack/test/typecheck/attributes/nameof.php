<?hh

class TakesStr implements HH\ClassAttribute {
  public function __construct(string $_): void {}
}

class TakesCls implements HH\ClassAttribute {
  public function __construct(classname<mixed> $_): void {}
}

<<TakesStr(nameof C)>>
class C {}

<<TakesCls(C::class)>>
class D {}
