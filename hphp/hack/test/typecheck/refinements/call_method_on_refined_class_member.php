<?hh

class A {
  public function m(): bool {
    return true;
  }
}

class Test {
  public ?A $a;

  public function test1(): void {
    if ($this->a !== null) {
      $this->a->m(); // ok
    }
  }

  public function test2(): void {
    if ($this->a !== null) {
      // TC used to raise wrong error here due to typechecking $this->a->m()
      // twice: once in `expr` and once in `condition`.
      if ($this->a->m()) {
      }
    }
  }
}
