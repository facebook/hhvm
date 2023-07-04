<?hh

class A {
  // repro bug: commented-out closing brace in extracted interface
  /*range-start*/
  public function foo(): void {
    // comment on first line
  }
  /*range-end*/
}
