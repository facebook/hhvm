<?hh

// Due to inheriting from an interface, this code fatals in HHVM.

interface I1 {
  public function meth(...$args): void;
}

class C1 implements I1 {
  public function meth(...$args): void {}
}

class C2 extends C1 {
  public function meth($x = null, ...): void {}
}
