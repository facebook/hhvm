<?hh

class A {
  public function __construct() {}
  public function break(B $b): string {
    return $b->get();
  }
  private function get(): int {
    return 42;
  }
}

class B extends A {
  public function __construct() {
    parent::__construct();
  }
  public function get(): string {
    return "-1";
  }
}
