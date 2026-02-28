<?hh

interface I {
  /**
   * This is a docblock 1
   */
  public function foo(): int;
}

interface J {
  /**
   * This is a docblock 2
   */
  public function foo(): int;
}

interface K {
  /**
   * This is a docblock 2
   */
  public function foo(): int;
}

class C implements I, J, K {
  public function foo(): int {
    return 42;
  }
}

function main(C $c): void {
  $c->foo();
  //  ^ hover-at-caret
}
