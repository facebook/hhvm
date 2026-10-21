<?hh

class A {
  private function do_not_inline_me(inout int $value): void {
    $value = 42;
  }

  public function foo(): void {
    $value = 0;
    // Inlining must not discard the write-back to the caller's variable.
    $this->/*range-start*/do_not_inline_me/*range-end*/(inout $value);
  }
}
