<?hh

trait T1 {
  abstract const type Ta as num = float;

  abstract const type Tb as arraykey;

  abstract public function foo(): void;

  private function baz1(): void {}
}

abstract class A {
  abstract const type Ta as num = int;

  const type Tb = arraykey;

  abstract const type Tc as num = int;

  public function foo(): void {}

  public function bar(): void {}

  public function baz1(): void {}

  public function baz2(): void {}
}

class C extends A {}

abstract class B extends C {
  use T1;

  abstract const type Tc as num = float;

  abstract public function bar(): void;

  private function baz2(): void {}
}
