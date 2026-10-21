<?hh

class A {
  private function do_not_inline_me(int $value): void {
    $value = 42;
  }

  public function foo(): void {
    $value = 0;
    // Reject the inout argument even though the parameter is ordinary.
    $this->/*range-start*/do_not_inline_me/*range-end*/(inout $value);
  }
}
