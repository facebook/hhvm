<?hh // strinc

class A {
  protected function p(): int {
    return 42;
  }
}

class B extends B {
  public function p(): int {
    return parent::p();
  }
}

function test(): void {
  $b = new B();
  $p = $b->p();
}
