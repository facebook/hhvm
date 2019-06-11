<?hh // strict
class A {

  private function get(): int {
    return 42;
  }
  public function foo(): int {
    if ($this is B) {
      $x = $this->get(); // we call get() on A
      print $this->bar(); // we call bar() on B
    } else {
      $x = -1;
    }
    return $x;
  }

}
class B extends A {
  public function get(): int {
    return 1;
  }
  public function bar(): int {
    return 5;
  }
}
