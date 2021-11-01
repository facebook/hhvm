<?hh

// Due to inheriting from an interface, this code fatals in HHVM.

interface I1 {
  public function meth(mixed ...$args): void;
}

class C1 implements I1 {
  public function meth(mixed ...$args): void {}
}

class C2 extends C1 {
  public function meth(mixed $x = null, mixed ...$args): void {}
}
