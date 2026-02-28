<?hh

class C {
  public function foo(shape(?"a" => int, ...) $sh): void {
    $sh["b"] = 3;
    $x =/*range-start*/$sh/*range-end*/;
  }
}
