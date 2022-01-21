<?hh

class A {
  public function foo(int $_): void {}
}

trait T1 {
  require extends A;
  public function bar(int $x): void {
    $this->foo($x);
  }
}

trait T2 {
  use T1;

  public function foo(string $_): void {}
  public function baz(string $x): void {
    $this->foo($x);
  }

}

class B extends A {
  use T2;
}
