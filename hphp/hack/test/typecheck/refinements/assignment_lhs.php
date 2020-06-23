<?hh // strict

class A {
  public ?B $b;
}

class B {
  public int $i = 42;
}

function test(A $a): void {
  $a->b = new B();
  $a->b->i = 42;
}
