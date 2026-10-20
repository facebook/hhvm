<?hh
//

class A {
  private function inline_me(int $x, int $y = 2): int {
    return $x + $y;
  }
  public function foo(): int {
    return $this->/*range-start*/inline_me/*range-end*/(1);
  }
}
