<?hh

class A {}

class C {
  public function foo(): void {
    $a = new A();
    $x =/*range-start*/ shape("a" => 2, "b" => $a) /*range-end*/;
  }
}
