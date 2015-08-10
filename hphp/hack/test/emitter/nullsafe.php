<?hh // strict

class A {
  public function __construct(public int $n) {}
  public function get(): int { return $this->n; }
}
class B {
  public function __construct(public ?A $a) {}
}

function blah(?A $a, ?B $b): void {
  var_dump($a?->get());
  var_dump($a?->n);
  var_dump($b?->a);
  var_dump($b?->a?->n);
}

function test(): void {
  $a = new A(5);
  blah($a, new B($a));
  blah(null, new B(null));
  blah(null, null);
}
