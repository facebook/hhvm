<?hh

class TheParent {
  public function foo(): void {}
}

class B extends /*range-start*/TheParent/*range-end*/ {}
