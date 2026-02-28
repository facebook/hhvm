<?hh

class A {
  // Ensure that we don't comment out the closing brace in extracted interface methods
  /*range-start*/
  public function foo(): void {
    // comment on first line
  }
  /*range-end*/
}
