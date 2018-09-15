<?hh // strict

type A<T> = (function(...): T);

type B<T> = ?(function(...): T);

type C = shape('x' => (function(...): void), 'y' => int);

class D {
  private ?(function(...): int) $fn;

  const ?(function(...): int) CONSTANT_FN = null;

  public function __construct() {}

  public function test(((function(...): int), int) $fn): void { // tuple param

  }

  public function test2((?(function(...): int), int) $fn): void { // tuple param

  }

  public function test3(vec<(function(...): int)> $fns): void {

  }

  public function test4(vec<?(function(...): int)> $fns): void {

  }

  public function test5(?(function(...): int) $fn): void {

  }

  public function test6(): ?(function(...): int) {
    return null;
  }
}
