<?hh // strict

class A {
  private function get(): int {
    return 42;
  }
  public function foo(): int {
    if ($this instanceof B) {
      $x = $this->get();
    } else {
      $x = -1;
    }
    return $x;
  }
}

class B extends A {}
