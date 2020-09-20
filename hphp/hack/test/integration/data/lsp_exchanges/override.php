<?hh

class MyParent {
  public function foo(): void {}
}

trait MyTrait {
  public function foo(): void {}
}

class MyChild extends MyParent {
  use MyTrait;

  <<__Override>>
  public function foo(): void {} // MyTrait should win over MyParent
}


class C1 {
  public static function bar(): void {}
}
class C2 extends C1 {
  <<__Override>>
  public static function bar(): void {}
}
class C3 extends C2 {
  <<__Override>>
  public static function bar(): void {} // Immediate parent should win.
}


interface I1 {
  public function quux(): void;
}
interface I2 extends I1 {
  <<__Override>>
  public function quux(): void;
}
