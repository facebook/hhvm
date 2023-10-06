<?hh

class A {
  public function foo(): void {
    // we should *not* offer the refactoring here
    new A/*range-start*//*range-end*/();
  }
}
