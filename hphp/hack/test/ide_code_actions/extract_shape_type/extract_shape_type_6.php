<?hh

class C {
  public function foo(vec<int> $v): void {
    $x =/*range-start*/ shape("a" => $v[0]) /*range-end*/;
  }
}
