<?hh

class A {
  public int $infinity = 0;
  public function cmp(A $other): int {
    return $this->infinity - $other->infinity;
  }
}
