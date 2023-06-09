<?hh

class C {
  public function foo(vec<int> $v): void {
    $x =/*range-start*/ shape("a" => shape("b" => $v)) /*range-end*/;
  }
}
