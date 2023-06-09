<?hh

class A {
  // not inlineable because it is non-private.
  // Our infra currently can't see beyond the current file, so we don't
  // know if hte function is called elsewhere (even statically).
  protected function do_not_inline_me(readonly int $_): string {
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
