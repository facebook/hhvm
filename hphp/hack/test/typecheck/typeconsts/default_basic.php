<?hh

abstract class AA {
  abstract const type T as arraykey = int;

  public function f(this::T $a): this::T {
    return $a;
  }
}

class A extends AA {}

class B extends AA {
  const type T = string;
}

function main(): void {
  $a = new A();
  $a->f(1);

  $b = new B();
  $b->f(1);
}
