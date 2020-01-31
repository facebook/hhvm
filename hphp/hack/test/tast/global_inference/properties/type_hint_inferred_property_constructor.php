<?hh //partial

class A {
  public function __construct(public $x = 3) {}
}

function foo(int $_): void {}

function bar(): void {
  $obj = new A(1);
  $x = $obj->x;
  foo($x);
}
