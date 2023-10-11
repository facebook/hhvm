<?hh

class A {
  public function __construct(private int $x) {}
  public function get(B $b): string {
    return $b->x;
  }
}

class B extends A {
  public function __construct(public string $x) {
    parent::__construct(42);
  }
}
