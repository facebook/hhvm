<?hh

class A {
  private function inline_me(string $x): string {
    $z_ = $x;
    $y = $x."\n";
    return $y;
  }

  public function foo(): void {
    $x = 1;
    "hello " . $this->/*range-start*/inline_me/*range-end*/("world");
    $z_ = 2;
  }
}
