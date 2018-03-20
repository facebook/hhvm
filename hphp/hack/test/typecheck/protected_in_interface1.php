<?hh // strict

interface I {
  protected function f(): int;
}

class C implements I {
  protected function f(): int {
    return 1;
  }
}
