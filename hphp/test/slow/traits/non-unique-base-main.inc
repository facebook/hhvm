<?hh

trait T {
  abstract protected function foo();
  public function main() { $this->foo(); }
}

class C extends B {
  use T;
}
