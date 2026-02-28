<?hh

class A {
  private function get(): int {
    return 42;
  }

  public function broken(C $c): string {
    return $c->get();
  }
}

class B extends A {}
class C extends B {
  public function get(): string {
    return "";
  }
}
