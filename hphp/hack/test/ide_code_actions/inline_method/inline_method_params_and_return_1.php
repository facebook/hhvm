<?hh

class A {
  private function inline_me(string $a, string $b): string {
    $x = 3;
    $z = $a . $b;
    return $z;
  }
  public function foo(): void {
    $x = 2;
    $s = $this->/*range-start*/inline_me/*range-end*/("a", "b");
  }
}
