<?hh

class A {
  // not inlineable because it has an inout param
  private function do_not_inline_me(inout string $_): string {
    if (1 < 2) {
      return "first return";
    }
    else {
      return "second return";
    }
  }
  public function foo(): void {
    $this->/*range-start*/do_not_inline_me/*range-end*/();
  }
}
