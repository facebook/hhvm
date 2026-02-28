<?hh

class C {
  public function foo(): void {
    $x =/*range-start*/ shape("a" => C::nonexistent()) /*range-end*/;
  }
}
