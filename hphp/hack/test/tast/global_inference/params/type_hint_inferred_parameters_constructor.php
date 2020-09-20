<?hh //partial

class A {
  public function __construct($x) {
    foo($x);
  }
}

function foo(int $x): void {}
