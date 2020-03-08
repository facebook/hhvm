<?hh // strict

namespace NS_override_on_return_type;

class C1 {};
class C2 extends C1 {};

interface I {}
class C3 implements I {}
class C4 implements I {}

class B {
  public function f1(): int { return 0; }
  public function f2(): int { return 0; }
  public function f3(): num { return 0; }
  public function f4(): num { return 0; }
  public function f5(): arraykey { return 0; }
  public function f6(): arraykey { return 0; }
  public function f7(): ?int { return 0; }
  public function f8(): ?int { return 0; }
  public function f9(): C1 { return new C1(); }
  public function f10(): C1 { return new C1(); }
  public function f11(): I { return new C3(); }
  public function f12(): I { return new C3(); }
}

class D extends B {
  public function f1(): int { return 100; }
//  public function f2(): float { return 100.2; }	// float return type incompatible with int
  public function f3(): int { return 0; }		// OK; int is compatible with num
  public function f4(): float { return 1.2; }	// OK; float is compatible with num
  public function f5(): int { return 0; }		// OK; int is compatible with arraykey
  public function f6(): string { return 'xxx'; }	// OK; string is compatible with arraykey
  public function f7(): ?int { return null; }
  public function f8(): int { return 0; }		// OK; int is compatible with ?int
  public function f9(): C1 { return new C1(); }
  public function f10(): C2 { return new C2(); }	// OK; C2 is compatible with C1
  public function f11(): I { return new C3(); }	// OK; C3 is compatible with I
  public function f12(): I { return new C4(); }	// OK; C4 is compatible with I
}


class GB<T> {
// Invalid return type: This is a value of generic type T. It is incompatible with an int

//  public function f13(): Vector<T> { return Vector {10, 20}; }
}

function main(): void {
  $b = new B();
  $b->f1();
  $b->f2();
  $b->f3();
  $b->f4();
  $b->f5();
  $b->f6();
  $b->f7();
  $b->f8();
  $b->f9();
  $b->f10();

  $d = new D();
  $d->f1();
  $d->f3();
  $d->f4();
  $d->f5();
  $d->f6();
  $d->f7();
  $d->f8();
  $d->f9();
  $d->f10();
}

/* HH_FIXME[1002] call to main in strict*/
main();
