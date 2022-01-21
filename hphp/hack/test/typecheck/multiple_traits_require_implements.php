<?hh

interface I {
  public function foo(int $_): void;
}

interface J {
  public function foo(string $_): void;
}

trait T1 {
  require implements I;
}

trait T2 {
  require implements J;
}

trait T {
  use T1;
  use T2;

  public function bar(): void {
    $this->foo("");
  }
}

class A implements J, I {
  use T;

  public function foo(int $_): void {}
}
