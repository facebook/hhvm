<?hh // strict

namespace NS_trait_implements_interface;

interface I1 {
  public function f(): void;
}

interface I2 {
  public function g(): void;
}

trait T implements I1, I2 {
  public function h(): void { $this->f(); }
}

class C {			// implicit implements clause
  use T;
  public function f(): void {}
  public function g(): void {}
}

class C2 implements I1, I2 {	// explicit implements clause
  use T;
  public function f(): void {}
  public function g(): void {}
}