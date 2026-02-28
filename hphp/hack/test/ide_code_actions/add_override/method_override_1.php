<?hh

class TheParent {
  public function foo(): void {}
  private function privFunc(): void {}
}

class B extends /*range-start*/TheParent/*range-end*/ {}
